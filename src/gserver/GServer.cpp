#include "GServer.h"

#include <assert.h>
#include <tinyserver/buffer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

#include "auth/PlayerSession.h"
#include "auth/Room.h"
#include "ChatClient.h"
#include "common/parseMessageHeader.h"
#include "proto/roompb.pb.h"

using namespace gserver;
using namespace gserver::protobuf;
using namespace tinyserver;

GServer::GServer(uint16_t port):
	_mainLoop(),
	_threadPool(&_mainLoop, 2),
	_tcpServer(_threadPool.getNextLoop(), InetAddress(port)),
	_started(false),
	_ctrl(),
	_userMgr(),
	_clientChat(_threadPool.getNextLoop(), &_userMgr)
{
}

GServer::~GServer()
{
}

void GServer::start()
{
	assert(_started == false);
	_started = true;

	_clientChat.start();

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
			} else {
				//!TODO 返回登录成功信息
				// RoomPBList allRoomPB;
				// for(auto room : _userMgr.roomMap()) {
				// 	auto roompb = allRoomPB.add_roompb();
				// 	roompb->CopyFrom(room.second->roomPB());
				// 	roompb->set_password("");
				// }
				// rspBuffer->append(allRoomPB.SerializePartialAsString());
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
				RoomPB roompb, chatRoomPB;
				roompb.ParseFromString(reqMsg);
				auto roomPtr = ps->createRoom(&roompb, &errmsg);
				bool isOk = false;
				// 创建聊天房间
				if(roomPtr) {
					RoomPB chatRoomPB;
					chatRoomPB.ParseFromString(reqMsg);
					isOk = _clientChat.createRoom(&chatRoomPB, &errmsg);
				}
				if(!isOk) {
					//!TODO 删除创建的房间
					TLOG_DEBUG(errmsg);
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				} else {	// 返回房间信息
					RoomPB retpb;
					roomPtr->toRoomPB(retpb);
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
					RoomPB retpb;
					roomPtr->toRoomPB(retpb);
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
					RoomPB retpb;
					roomPtr->toRoomPB(retpb);
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
	case Command::ROOMLIST:
		{
			auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
			if(ps) {
				RoomPBList allRoomPB;
				for(auto room : _userMgr.roomMap()) {
					auto roompb = allRoomPB.add_roompb();
					roompb->set_id(room.second->id());
					roompb->set_name(room.second->name());
				}
				rspBuffer->append(allRoomPB.SerializePartialAsString());
			} else {
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			}
		}
		break;
	case Command::ROOMINFO:
		{
			auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
			if(ps) {
				RoomPB roompb;
				roompb.ParseFromString(reqMsg);

				auto roomPtr = _userMgr.getRoomById(roompb.id());
				if(roomPtr) {
					RoomPB retpb;
					roomPtr->toRoomPB(retpb);
					PlayerPB* playerpb = new PlayerPB();
					playerpb->set_id(ps->playerPB().id());
					playerpb->set_name(ps->playerPB().name());
					retpb.set_allocated_owner(playerpb);
					rspBuffer->append(retpb.SerializePartialAsString());
				} else {
					header.rspcode = RspCode::ERROR;
					rspBuffer->append("房间不存在");
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