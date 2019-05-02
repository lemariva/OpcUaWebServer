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

#ifndef __OpcUaWebServer_RequestHeader_h__
#define __OpcUaWebServer_RequestHeader_h__

#include <boost/property_tree/ptree.hpp>

namespace OpcUaWebServer
{

	class RequestHeader
	{
	  public:
		RequestHeader(void);
		RequestHeader(const RequestHeader& RequestHeader);
		RequestHeader(
		    const std::string& messageType,
			const std::string& clientHandle,
			const std::string& sessionId
		);
		virtual ~RequestHeader(void);

		std::string& messageType(void);
		std::string& clientHandle(void);
		std::string& sessionId(void);

		bool jsonEncode(boost::property_tree::ptree& pt);
		bool jsonDecode(boost::property_tree::ptree& pt);

	  private:
		std::string messageType_;
		std::string clientHandle_;
		std::string sessionId_;
	};

}

#endif
