/*
   Copyright 2015-2019 Kai Huebl (kai@huebl-sgh.de)

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

#include "OpcUaStackCore/Network/TCPAcceptor.h"
#include "OpcUaStackCore/Network/TCPConnection.h"
#include "OpcUaWebServer/WebSocket/WebSocketConfig.h"
#include "OpcUaWebServer/WebSocket/WebSocketChannel.h"
#include "OpcUaWebServer/WebSocket/WebSocketMessage.h"

namespace OpcUaWebServer
{

	class WebSocketServerBase
	{
	  public:
		typedef enum {
			OP_CONTINUATION_FRAME = 0,
			OP_TEXT_FRAME = 1,
			OP_BINARY_FRAME = 2,
			OP_CLOSE_FRAME = 8,
			OP_PING_FRAME = 9
		} OpCode;

		typedef std::function<void (WebSocketMessage& webSocketMessag)> ReceiveMessageCallback;

		WebSocketServerBase(WebSocketConfig* webSocketConfig);
		virtual ~WebSocketServerBase(void);

		void disconnect(uint32_t channelId);
		void receiveMessageCallback(const ReceiveMessageCallback& receiveMessageCallback);
		bool sendMessage(WebSocketMessage& webSocketMessage);

	  protected:
		void receiveHandshake(WebSocketChannel* webSocketChannel);

	  protected:
		void initWebSocketChannel(WebSocketChannel* webSocketChannel);
		void cleanupWebSocketChannel(WebSocketChannel* webSocketChannel);
		void closeWebSocketChannel(WebSocketChannel* httpChannel);

		//
		// handle handshake
		//
		void handleReceiveHandshakeHeaderTimeout(WebSocketChannel* webSocketChannel);
		void handleReceiveHandshakeHeader(const boost::system::error_code& error, std::size_t bytes_transfered, WebSocketChannel* webSocketChannel);
		void handleReceiveHandshakeContentTimeout(WebSocketChannel* webSocketChannel);
		void handleReceiveHandshakeContent(const boost::system::error_code& error, std::size_t bytes_transfered, WebSocketChannel* webSocketChannel);
		void handleIdleTimeout(WebSocketChannel* webSocketChannel);
		void processHandshake(WebSocketChannel* httpChannel, WebSocketRequest& webSocketRequest);
		void handleWriteComplete(const boost::system::error_code& error, WebSocketChannel* httpChannel);
		int base64Encode(const unsigned char* buffer, size_t length, char** b64text);

		void requestTimeoutWebSocketChannel(WebSocketChannel* webSocketChannel, const std::string& location);


		//
		// handle receive message
		//
		void receiveMessage(WebSocketChannel* webSocketChannel);
		void handleReceiveMessageHeader(const boost::system::error_code& error, std::size_t bytes_transfered, WebSocketChannel* webSocketChannel);
		void handleReceiveMessageLength2Timeout(WebSocketChannel* webSocketChannel);
		void handleReceiveMessageLength2(const boost::system::error_code& error, std::size_t bytes_transfered, WebSocketChannel* webSocketChannel);
		void handleReceiveMessageContentTimeout(WebSocketChannel* webSocketChannel);
		void handleReceiveMessageContent(const boost::system::error_code& error, std::size_t bytes_transfered, WebSocketChannel* webSocketChannel);

		void idleTimeoutWebSocketChannel(WebSocketChannel* webSocketChannel, const std::string& location);


		//
		// handle send message
		//
		bool sendMessage(WebSocketMessage& webSocketMessage, WebSocketChannel* webSocketChannel);
		void handleWriteMessageComplete(const boost::system::error_code& error, WebSocketChannel* httpChannel);



		WebSocketConfig* webSocketConfig_;
		TCPAcceptor tcpAcceptor_;

		ReceiveMessageCallback receiveMessageCallback_;

		boost::mutex mutex_;
		WebSocketChannel::Map webSocketChannelMap_;
	};

}
