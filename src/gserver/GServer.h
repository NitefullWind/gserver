#ifndef GSERVER_GSERVER_H
#define GSERVER_GSERVER_H

#include "gserver.h"
#include "Controller.h"

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

		std::shared_ptr<PlayerSession> getLoggedPlayer(const tinyserver::TcpConnectionPtr& tcpConnPtr, std::string *errmsg = nullptr);

	private:
		Controller _ctrl;
	};
}

#endif // GSERVER_GSERVER_H