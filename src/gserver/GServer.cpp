#include "GServer.h"

#include <assert.h>
#include <tinyserver/buffer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

#include "auth/PlayerSession.h"
#include "auth/Room.h"
#include "common/parseMessageHeader.h"
#include "proto/roompb.pb.h"

using namespace gserver;
using namespace tinyserver;

GServer::GServer(uint16_t port):
	_mainLoop(),
	_threadPool(&_mainLoop, 2),
	_tcpServer(_threadPool.getNextLoop(), InetAddress(port)),
	_clientChat(_threadPool.getNextLoop(), InetAddress("127.0.0.1", 8087)),
	_started(false)
{
}

GServer::~GServer()
{
}

void GServer::start()
{
	assert(_started == false);
	_started = true;

	_clientChat.setRetry(true);
	_clientChat.setConnectionCallback([](const TcpConnectionPtr& connPtr) {
		TLOG_INFO("连接到聊天服务器:" << connPtr->peerAddress().toHostPort());
		PlayerPB admin_gserver;
		admin_gserver.set_name("Admin Gserver");
		sendMessageToConnection(connPtr, Command::LOGIN, 0, admin_gserver.SerializePartialAsString());
	});
	_clientChat.setMessageCallback([](const TcpConnectionPtr& connPtr, Buffer *buffer) {
		
	});
	_clientChat.setDisconnectionCallback([](const TcpConnectionPtr& connPtr) {
		TLOG_INFO("已断开聊天服务器:" << connPtr->peerAddress().toHostPort());
	});
	_clientChat.connect();

	_tcpServer.setIOThreadNum(2);
	_tcpServer.setConnectionCallback(std::bind(&GServer::onNewConnection, this, std::placeholders::_1));
	_tcpServer.setDisconnectionCallback(std::bind(&GServer::onDisconnection, this, std::placeholders::_1));
	_tcpServer.setMessageCallback([this](const TcpConnectionPtr& connPtr, Buffer *buffer) {
		MessageHeader header = {0, Command::INVILID, 0, 0, RspCode::SUCCESS, 0};
		Buffer rspBuffer;
		std::string errmsg;
		if(parseMessageHeader(buffer, header, &errmsg)) {
			this->processRequest(header, connPtr, buffer->read(header.datalen), &rspBuffer);
		} else {
			TLOG_INFO(errmsg);
		}
		header.datalen = static_cast<uint32_t>(rspBuffer.readableBytes());
		writeHeaderToBuffer(&rspBuffer, header);
		connPtr->send(&rspBuffer);
	});
	_tcpServer.start();

	_threadPool.start();
	_mainLoop.loop();
}

void GServer::onNewConnection(const tinyserver::TcpConnectionPtr& tcpConnPtr)
{
	TLOG_DEBUG("connection:" << tcpConnPtr->peerAddress().toHostPort());
}

void GServer::onDisconnection(const tinyserver::TcpConnectionPtr& tcpConnPtr)
{
	TLOG_DEBUG("disconnection:" << tcpConnPtr->peerAddress().toHostPort());
	_userMgr.logout(tcpConnPtr->id());
}

void GServer::processRequest(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg, Buffer *rspBuffer)
{
	std::string errmsg;
	switch (header.cmd)
	{
	case Command::LOGIN:
		{
			if(!_userMgr.login(tcpConnPtr, reqMsg, &errmsg)) {
				TLOG_DEBUG(errmsg);
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			} else {	// 返回房间列表
				RoomPBList allRoomPB;
				for(auto room : _userMgr.roomMap()) {
					auto roompb = allRoomPB.add_roompb();
					roompb->CopyFrom(room.second->roomPB());
					roompb->set_password("");
				}
				rspBuffer->append(allRoomPB.SerializePartialAsString());
			}
		}
		break;
	case Command::LOGOUT:
		{
			if(!_userMgr.logout(tcpConnPtr->id(), &errmsg)) {
				TLOG_DEBUG(errmsg);
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			}
		}
		break;
	case Command::CREATEROOM:
		{
			auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
			if(ps) {
				RoomPB roompb;
				roompb.ParseFromString(reqMsg);
				auto roomPtr = ps->createRoom(&roompb, &errmsg);
				if(!roomPtr) {
					TLOG_DEBUG(errmsg);
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				} else {	// 返回房间信息
					auto retpb = roomPtr->roomPB();
					retpb.set_password("");
					rspBuffer->append(retpb.SerializePartialAsString());
				}
			} else {
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			}
		}
		break;
	case Command::UPDATEROOM:
		{
			auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
			if(ps) {
				RoomPB roompb;
				roompb.ParseFromString(reqMsg);
				std::string errmsg;
				auto roomPtr = ps->updateRoom(&roompb, &errmsg);
				if(!roomPtr) {
					TLOG_DEBUG(errmsg);
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				}  else {	// 返回房间信息
					auto retpb = roomPtr->roomPB();
					retpb.set_password("");
					rspBuffer->append(retpb.SerializePartialAsString());
				}
			} else {
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			}
		}
		break;
	case Command::JOINROOM:
		{
			auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
			if(ps) {
				RoomPB roompb;
				roompb.ParseFromString(reqMsg);
				auto roomPtr = ps->joinRoom(roompb.id(), &errmsg);
				if(!roomPtr) {
					TLOG_DEBUG(errmsg);
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				} else {	// 返回房间信息
					auto retpb = roomPtr->roomPB();
					retpb.set_password("");
					rspBuffer->append(retpb.SerializePartialAsString());
				}
			} else {
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			}
		}
		break;
	case Command::EXITROOM:
		{
			auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
			if(ps) {
				RoomPB roompb;
				roompb.ParseFromString(reqMsg);
				if(ps->exitRoom(roompb.id(), &errmsg)) {
					TLOG_DEBUG("Exit room success.")
				} else {
					TLOG_DEBUG("Exit room failed, " << errmsg);
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				}
			} else {
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			}
		}
		break;
	
	default:
		TLOG_DEBUG("Invalid Command!");
		header.rspcode = RspCode::ERROR;
		rspBuffer->append("Invalid Command!");
		break;
	}
}