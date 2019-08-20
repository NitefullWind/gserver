#include "ChatServer.h"
#include "proto/message.pb.h"

#include <tinyserver/buffer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

using namespace gserver;
using namespace tinyserver;

ChatServer::ChatServer()
{
}

ChatServer::~ChatServer()
{
}


void ChatServer::onNewConnection(const tinyserver::TcpConnectionPtr& tcpConnPtr)
{

}

void ChatServer::onDisconnection(const tinyserver::TcpConnectionPtr& tcpConnPtr)
{

}

void ChatServer::processRequest(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg, Buffer *rspBuffer)
{
	std::string errmsg;
	switch (header.cmd)
	{
		case Command::SENDMSG:
		{
			MessagePB msgPB;
			msgPB.ParseFromString(reqMsg);
			TLOG_DEBUG("recv message:")
			TLOG_DEBUG("timestamp:" << msgPB.timestamp());
			TLOG_DEBUG("messageType:" << msgPB.messagetype());
			TLOG_DEBUG("dataType:" << msgPB.datatype());
			TLOG_DEBUG("data:" << msgPB.data());
			rspBuffer->append(msgPB.SerializePartialAsString());
			break;
		}
		default:
		{
			header.rspcode = RspCode::ERROR;
			rspBuffer->append("Invalid Command!");
		}
		break;
	}
}