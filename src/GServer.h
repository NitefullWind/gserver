#ifndef GSERVER_GSERVER_H
#define GSERVER_GSERVER_H

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
	enum Command
	{
		INVILID = 0,
		LOGIN = 1,
		LOGOUT = 2,
		CREATEROOM = 100,
		UPDATEROOM = 101,
		JOINROOM = 102,
		EXITROOM = 103,
	};

	typedef struct
	{
		uint16_t flag;
		uint16_t cmd;
		uint16_t datalen;
		
	} RequestHeader;

	#define RequestHeaderFlag 0x4753

	class PlayerSession;

	class GServer
	{
	public:
		GServer();
		~GServer();

		void onNewConnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void onDisconnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void onNewMessage(RequestHeader header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& message);

		std::shared_ptr<PlayerSession> getLoggedPlayer(const tinyserver::TcpConnectionPtr& tcpConnPtr);

	private:
		Controller _ctrl;
	};
}

#endif // GSERVER_GSERVER_H