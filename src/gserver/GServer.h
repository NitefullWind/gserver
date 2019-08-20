#ifndef GSERVER_GSERVER_H
#define GSERVER_GSERVER_H

#include "gserver.h"
#include "Controller.h"
#include "auth/UserManager.h"

#include <memory>

namespace tinyserver
{
class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
}

namespace gserver
{
	class PlayerSession;

	class GServer
	{
	public:
		GServer();
		~GServer();

		void onNewConnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void onDisconnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void processRequest(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg, tinyserver::Buffer *rspBuffer);
	private:
		Controller _ctrl;
		UserManager _userMgr;
	};
}

#endif // GSERVER_GSERVER_H