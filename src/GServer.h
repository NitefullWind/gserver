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

	enum RspCode
	{
		ERROR = 0,
		SUCCESS = 1,
	};

	typedef struct
	{
		uint16_t flag;
		uint16_t cmd;
		uint32_t reqid;
		uint16_t datalen;
		uint16_t rspcode;
		uint32_t clientversion;
	} MessageHeader;

	#define MessageHeaderFlag 0x4753

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