#ifndef GSERVER_CHATSERVER_H
#define GSERVER_CHATSERVER_H

#include <memory>

#include "auth/UserManager.h"
#include "gserver.h"
#include "proto/message.pb.h"
#include "proto/playerpb.pb.h"
#include "proto/roompb.pb.h"

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

		void init();

		void onNewConnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void onDisconnection(const tinyserver::TcpConnectionPtr& tcpConnPtr);
		void processRequest(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg, tinyserver::Buffer *rspBuffer);
	private:
		UserManager _userMgr;

		bool sendMsgToGroup(const RoomPB& roomPB, const MessagePB& msgPB, std::string *errmsg = nullptr);
		bool sendMsgToGroup(int groupId, const MessagePB& msgPB, std::string *errmsg = nullptr);
		bool sendMsgToGroup(int groupId, const std::string& fromUserId, const std::string& msgPBStr, std::string *errmsg = nullptr);
		bool sendMsgToUser(const PlayerPB& playerPB, const MessagePB& msgPB, std::string *errmsg = nullptr);
		bool sendMsgToUser(const std::string& userId, const MessagePB& msgPB, std::string *errmsg = nullptr);
		bool sendMsgToUser(const std::string& userId, const std::string& msgPBStr, std::string *errmsg = nullptr);

		void createDefaultChatRoom();
	};
}

#endif // GSERVER_CHATSERVER_H