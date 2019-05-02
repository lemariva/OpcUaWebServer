/*
   Copyright 2019 Kai Huebl (kai@huebl-sgh.de)

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

#include <OpcUaWebServer/WebGateway/LogoutRequest.h>
#include "OpcUaStackCore/Base/Log.h"

using namespace OpcUaStackCore;

namespace OpcUaWebServer
{

	LogoutRequest::LogoutRequest(void)
	: sessionId_("")
	{
	}

	LogoutRequest::~LogoutRequest(void)
	{
	}

	std::string&
	LogoutRequest::sessionId(void)
	{
		return sessionId_;
	}


	bool
	LogoutRequest::jsonEncode(boost::property_tree::ptree& pt)
	{
		pt.put("sessionId", sessionId_);
		return true;
	}

	bool
	LogoutRequest::jsonDecode(boost::property_tree::ptree& pt)
	{
		// get discovery url from json message
		boost::optional<std::string> sessionId = pt.get_optional<std::string>("sessionId");
		if (!sessionId) {
			Log(Error, "message body do not contain session id");
			return false;
		}
		sessionId_ = *sessionId;

		return true;
	}

}

