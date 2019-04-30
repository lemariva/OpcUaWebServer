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

#ifndef __OpcUaWebServer_ClientManager_h__
#define __OpcUaWebServer_ClientManager_h__

#include <OpcUaWebServer/WebGateway/RequestHeader.h>
#include "OpcUaStackCore/Utility/IOThread.h"
#include "OpcUaStackCore/Certificate/CryptoManager.h"
#include "OpcUaWebServer/WebSocket/WebSocketMessage.h"

using namespace OpcUaStackCore;

namespace OpcUaWebServer
{

	class ClientManager
	{
	  public:
		typedef std::function<void (WebSocketMessage& webSocketMessag)> SendMessageCallback;

		ClientManager(void);
		virtual ~ClientManager(void);

		bool startup(
			IOThread::SPtr ioThread,
			CryptoManager::SPtr& cryptoManager
		);
		bool shutdown(void);

		void sendMessageCallback(const SendMessageCallback& sendMessageCallback);
		void receiveMessage(WebSocketMessage& webSocketMessag);

	  private:
		void sendErrorResponse(
			uint32_t channelId,
			RequestHeader& requestHeader,
			OpcUaStatusCode statusCode
		);

		SendMessageCallback sendMessageCallback_;
	};

}

#endif
