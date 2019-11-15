#ifndef GSERVER_GSERVER_H
#define GSERVER_GSERVER_H

#include "gserver.h"
#include "Controller.h"
#include "auth/UserManager.h"
#include "ChatClient.h"

#include <memory>
#include <tinyserver/eventLoop.h>
#include <tinyserver/eventLoopThreadPool.h>
#include <tinyserver/tcp/TcpClient.h>
#include <tinyserver/tcp/tcpServer.h>

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
		GServer(uint16_t port);
		~GServer();

		void start();

		void onNewConnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void onDisconnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void processRequest(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg, tinyserver::Buffer *rspBuffer);
	private:
		tinyserver::EventLoop _mainLoop;
		tinyserver::EventLoopThreadPool _threadPool;
		tinyserver::TcpServer _tcpServer;
		bool _started;
		Controller _ctrl;
		UserManager _userMgr;
		ChatClient _clientChat;
	};
}

#endif // GSERVER_GSERVER_H