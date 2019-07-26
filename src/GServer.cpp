#include "GServer.h"

#include <assert.h>
#include <tinyserver/buffer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

#include "PlayerSession.h"
#include "Room.h"
#include "proto/roompb.pb.h"

using namespace gserver;
using namespace tinyserver;

GServer::GServer()
{
}

GServer::~GServer()
{
}

void GServer::onNewConnection(const tinyserver::TcpConnectionPtr& tcpConnPtr)
{
	TLOG_DEBUG("connection:" << tcpConnPtr->peerAddress().toHostPort());
}

void GServer::onDisconnection(const tinyserver::TcpConnectionPtr& tcpConnPtr)
{
	TLOG_DEBUG("disconnection:" << tcpConnPtr->peerAddress().toHostPort());
	_ctrl.logout(tcpConnPtr->id());
}

void GServer::processRequest(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg, Buffer *rspBuffer)
{
	std::string errmsg;
	switch (header.cmd)
	{
	case Command::LOGIN:
		{
			std::shared_ptr<PlayerSession> ps(new PlayerSession(&_ctrl, tcpConnPtr));
			PlayerPB playerpb;
			playerpb.ParseFromString(reqMsg);
			playerpb.set_id(tcpConnPtr->id());	//将TcpConnection的id设为用户id
			ps->setPlayerPB(std::move(playerpb));
			if(!_ctrl.login(ps, &errmsg)) {
				TLOG_DEBUG(errmsg);
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			} else {	// 返回房间列表
				RoomPBList allRoomPB;
				for(auto room : _ctrl.roomMap()) {
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
			if(!_ctrl.logout(tcpConnPtr->id(), &errmsg)) {
				TLOG_DEBUG(errmsg);
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			}
		}
		break;
	case Command::CREATEROOM:
		{
			auto ps = getLoggedPlayer(tcpConnPtr, &errmsg);
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
			auto ps = getLoggedPlayer(tcpConnPtr, &errmsg);
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
			auto ps = getLoggedPlayer(tcpConnPtr, &errmsg);
			if(ps) {
				RoomPB roompb;
				roompb.ParseFromString(reqMsg);
				auto roomPtr = ps->joinRoom(&roompb, &errmsg);
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
			auto ps = getLoggedPlayer(tcpConnPtr, &errmsg);
			if(ps) {
				if(ps->exitRoom(&errmsg)) {
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

std::shared_ptr<PlayerSession> GServer::getLoggedPlayer(const tinyserver::TcpConnectionPtr& tcpConnPtr, std::string *errmsg)
{
	auto ps =  _ctrl.getPlayerSessionById(tcpConnPtr->id());
	if(ps == nullptr) {
		if(errmsg) {
			*errmsg = "用户未登陆";
		}
		TLOG_DEBUG(*errmsg);
	}
	return ps;
}