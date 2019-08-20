#include "GServer.h"

#include <assert.h>
#include <tinyserver/buffer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

#include "auth/PlayerSession.h"
#include "auth/Room.h"
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
			auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
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