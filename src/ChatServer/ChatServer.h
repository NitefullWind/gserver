#ifndef GSERVER_CHATSERVER_H
#define GSERVER_CHATSERVER_H

#include <memory>

#include "gserver.h"
#include "auth/UserManager.h"

namespace tinyserver
{
class Buffer;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
}

namespace gserver
{
	class ChatServer
	{
	public:
		ChatServer();
		~ChatServer();

		void onNewConnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void onDisconnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void processRequest(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg, tinyserver::Buffer *rspBuffer);
	private:
		UserManager _userMgr;
	};
}

#endif // GSERVER_CHATSERVER_H