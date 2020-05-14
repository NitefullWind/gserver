#ifndef GSERVER_CHATSERVER_H
#define GSERVER_CHATSERVER_H

#include <memory>

#include "auth/UserManager.h"
#include "gserver.h"
#include "proto/chatmsgpb.pb.h"
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

		bool sendMsgToGroup(const protobuf::RoomPB& roomPB, const protobuf::ChatMsgPB& msgPB, std::string *errmsg = nullptr);
		bool sendMsgToGroup(int32_t groupId, const protobuf::ChatMsgPB& msgPB, std::string *errmsg = nullptr);
		bool sendMsgToGroup(int32_t groupId, const std::string& senderId, const std::string& msgPBStr, std::string *errmsg = nullptr);
		bool sendMsgToUser(const protobuf::PlayerPB& playerPB, const protobuf::ChatMsgPB& msgPB, std::string *errmsg = nullptr);
		bool sendMsgToUser(const std::string& userId, const protobuf::ChatMsgPB& msgPB, std::string *errmsg = nullptr);
		bool sendMsgToUser(const std::string& userId, const std::string& msgPBStr, std::string *errmsg = nullptr);

		void createDefaultChatRoom();
	};
}

#endif // GSERVER_CHATSERVER_H