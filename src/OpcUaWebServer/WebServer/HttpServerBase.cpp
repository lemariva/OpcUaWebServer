/*
   Copyright 2015 Kai Huebl (kai@huebl-sgh.de)

   Lizenziert gemäß Apache Licence Version 2.0 (die „Lizenz“); Nutzung dieser
   Datei nur in Übereinstimmung mit der Lizenz erlaubt.
   Eine Kopie der Lizenz erhalten Sie auf http://www.apache.org/licenses/LICENSE-2.0.

   Sofern nicht gemäß geltendem Recht vorgeschrieben oder schriftlich vereinbart,
   erfolgt die Bereitstellung der im Rahmen der Lizenz verbreiteten Software OHNE
   GEWÄHR ODER VORBEHALTE – ganz gleich, ob ausdrücklich oder stillschweigend.

   Informationen über die jeweiligen Bedingungen für Genehmigungen und Einschränkungen
   im Rahmen der Lizenz finden Sie in der Lizenz.

   Autor: Kai Huebl (kai@huebl-sgh.de)

 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "OpcUaStackCore/Base/Log.h"
#include "OpcUaWebServer/WebServer/HttpServerBase.h"

using namespace OpcUaStackCore;

namespace OpcUaWebServer
{

	HttpServerBase::HttpServerBase(HttpConfig* httpConfig)
	: httpConfig_(httpConfig)
	, tcpAcceptor_(httpConfig->ioThread()->ioService()->io_service(), httpConfig->address(), httpConfig->port())
	, httpServerIfMap_()
	{
	}

	HttpServerBase::~HttpServerBase(void)
	{
	}

	bool
	HttpServerBase::addHttpServerIf(const std::string& method, HttpServerIf* httpServerIf)
	{
		std::string methodTmp = method;
		boost::to_upper(methodTmp);

		HttpServerIfMap::iterator it;
		it = httpServerIfMap_.find(methodTmp);
		if (it != httpServerIfMap_.end()) return false;
		httpServerIfMap_.insert(std::make_pair(method, httpServerIf));
		return true;
	}

	void
	HttpServerBase::receiveRequest(HttpChannel* httpChannel)
	{
		// start timer
		httpChannel->slotTimerElement_->expireFromNow(httpConfig_->requestTimeout());
		httpChannel->slotTimerElement_->timeoutCallback(boost::bind(&HttpServerBase::handleReceiveRequestHeaderTimeout, this, httpChannel));
		httpConfig_->ioThread()->slotTimer()->start(httpChannel->slotTimerElement_);

		// read data until \r\n\r\n
		httpChannel->async_read_until(
			httpChannel->recvBuffer_,
			boost::bind(&HttpServerBase::handleReceiveRequestHeader, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, httpChannel),
			"\r\n\r\n"
		);
	}

	void
	HttpServerBase::handleReceiveRequestHeaderTimeout(HttpChannel* httpChannel)
	{
		timeoutHttpChannel(httpChannel, "request header");
	}

	void
	HttpServerBase::handleReceiveRequestHeader(const boost::system::error_code& error, std::size_t bytes_transfered, HttpChannel* httpChannel)
	{
		if (httpChannel->timeout_) {
			closeHttpChannel(httpChannel);
			return;
		}

		// stop request timer
		httpConfig_->ioThread()->slotTimer()->stop(httpChannel->slotTimerElement_);

		// handle error
		if (error) {
			Log(Debug, "HttpServer receive request header error; close channel")
				.parameter("Address", httpChannel->partner_.address().to_string())
				.parameter("Port", httpChannel->partner_.port())
				.parameter("ChannelId", httpChannel->channelId_);

			closeHttpChannel(httpChannel);
			return;
		}

		size_t numAdditionalBytes = httpChannel->recvBuffer_.size() - bytes_transfered;
		std::istream is(&httpChannel->recvBuffer_);

		//
		// read request header
		//
		if (!httpChannel->httpRequest_.decodeRequestHeader(is)) {
			Log(Debug, "HttpServer decode request error; close channel")
				.parameter("Address", httpChannel->partner_.address().to_string())
				.parameter("Port", httpChannel->partner_.port());

			closeHttpChannel(httpChannel);
			return;
		}

		//
		// read request content
		//
		std::string contentLengthString;
		if (!httpChannel->httpRequest_.getHeaderPara("Content-Length", contentLengthString)) {
			processRequest(httpChannel, httpChannel->httpRequest_);
			return;
		}

		bool success;
		size_t contentLength;
		try {
			contentLength = boost::lexical_cast<size_t>(contentLengthString);
		} catch(boost::bad_lexical_cast&) {
			success = false;
		}

		if (!success) {
			Log(Debug, "HttpServer content length error in request; close channel")
				.parameter("Address", httpChannel->partner_.address().to_string())
				.parameter("Port", httpChannel->partner_.port());

			closeHttpChannel(httpChannel);
			return;
		}

		// start request timer
		httpChannel->slotTimerElement_->expireFromNow(httpConfig_->requestTimeout());
		httpChannel->slotTimerElement_->timeoutCallback(boost::bind(&HttpServerBase::handleReceiveRequestContentTimeout, this, httpChannel));
		httpConfig_->ioThread()->slotTimer()->start(httpChannel->slotTimerElement_);

		httpChannel->async_read_exactly(
			httpChannel->recvBuffer_,
			boost::bind(&HttpServerBase::handleReceiveRequestContent, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, httpChannel),
			contentLength-numAdditionalBytes
		);
	}

	void
	HttpServerBase::handleReceiveRequestContentTimeout(HttpChannel* httpChannel)
	{
		timeoutHttpChannel(httpChannel, "request Content");
	}

	void
	HttpServerBase::handleReceiveRequestContent(const boost::system::error_code& error, std::size_t bytes_transfered, HttpChannel* httpChannel)
	{
		if (httpChannel->timeout_) {
			closeHttpChannel(httpChannel);
			return;
		}

		// stop request timer
		httpConfig_->ioThread()->slotTimer()->stop(httpChannel->slotTimerElement_);

		if (error) {
			Log(Debug, "HttpServer receive request content error; close channel")
				.parameter("Address", httpChannel->partner_.address().to_string())
				.parameter("Port", httpChannel->partner_.port())
				.parameter("ChannelId", httpChannel->channelId_);

			closeHttpChannel(httpChannel);
			return;
		}

		//
		// read content
		//
		processRequest(httpChannel, httpChannel->httpRequest_);
	}

	void
	HttpServerBase::processRequest(HttpChannel* httpChannel, HttpRequest& httpRequest)
	{
		// determine request method
		std::string method = httpRequest.method();
		boost::to_upper(method);

		// find http server interface for method
		HttpServerIfMap::iterator it;
		it = httpServerIfMap_.find(method);
		if (it == httpServerIfMap_.end()) {
			httpChannel->httpResponse_.statusCode(404);
			httpChannel->httpResponse_.statusString("Not Found");

			std::ostream os(&httpChannel->sendBuffer_);
			httpChannel->httpResponse_.encodeRequestHeader(os);
			httpChannel->async_write(
				httpChannel->sendBuffer_,
				boost::bind(&HttpServerBase::handleWriteComplete, this, boost::asio::placeholders::error, httpChannel)
			);
			return;
		}

		HttpServerIf* httpServerIf = it->second;
		httpServerIf->request(httpChannel->httpRequest_, httpChannel->httpResponse_);

		// send response
		std::ostream os(&httpChannel->sendBuffer_);
		httpChannel->httpResponse_.encodeRequestHeader(os);
		httpChannel->async_write(
			httpChannel->sendBuffer_,
			boost::bind(&HttpServerBase::handleWriteComplete, this, boost::asio::placeholders::error, httpChannel)
		);

		return;
	}

	void
	HttpServerBase::handleWriteComplete(const boost::system::error_code& error, HttpChannel* httpChannel)
	{
		if (error) {
			Log(Debug, "HttpServer send response error; close channel")
				.parameter("Address", httpChannel->partner_.address().to_string())
				.parameter("Port", httpChannel->partner_.port())
				.parameter("ChannelId", httpChannel->channelId_);

			closeHttpChannel(httpChannel);
			return;
		}

		httpChannel->close();
		delete httpChannel;
	}

	void
	HttpServerBase::closeHttpChannel(HttpChannel* httpChannel)
	{
		httpChannel->close();
		delete httpChannel;
	}

	void
	HttpServerBase::timeoutHttpChannel(HttpChannel* httpChannel, const std::string& location)
	{
		Log(Debug, "HttpServer timeout; close channel")
			.parameter("Address", httpChannel->partner_.address().to_string())
			.parameter("Port", httpChannel->partner_.port())
			.parameter("Location", location)
			.parameter("ChannelId", httpChannel->channelId_);

		httpChannel->timeout_ = true;
		httpChannel->cancel();
	}

}
