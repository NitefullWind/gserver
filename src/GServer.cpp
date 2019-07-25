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

void GServer::onNewMessage(RequestHeader header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& message)
{
	switch (header.cmd)
	{
	case Command::LOGIN:
		{
			std::shared_ptr<PlayerSession> ps(new PlayerSession(&_ctrl, tcpConnPtr));
			PlayerPB playerpb;
			playerpb.ParseFromString(message);
			playerpb.set_id(tcpConnPtr->id());	//将TcpConnection的id设为用户id
			ps->setPlayerPB(std::move(playerpb));
			std::string errmsg;
			if(!_ctrl.login(ps, &errmsg)) {
				TLOG_DEBUG(errmsg);
			}
		}
		break;
	case Command::LOGOUT:
		{
			std::string errmsg;
			if(!_ctrl.logout(tcpConnPtr->id(), &errmsg)) {
				TLOG_DEBUG(errmsg);
			}
		}
		break;
	case Command::CREATEROOM:
		{
			auto ps = getLoggedPlayer(tcpConnPtr);
			if(ps) {
				RoomPB roompb;
				roompb.ParseFromString(message);
				std::string errmsg;
				bool isOk = ps->createRoom(&roompb, &errmsg);
				if(!isOk) {
					TLOG_DEBUG(errmsg);
				}
			}
		}
		break;
	case Command::UPDATEROOM:
		{
			auto ps = getLoggedPlayer(tcpConnPtr);
			if(ps) {
				RoomPB roompb;
				roompb.ParseFromString(message);
				ps->updateRoom(&roompb);
			}
		}
		break;
	case Command::JOINROOM:
		{
			auto ps = getLoggedPlayer(tcpConnPtr);
			if(ps) {
				RoomPB roompb;
				roompb.ParseFromString(message);
				std::string errmsg;
				bool isOk = ps->joinRoom(&roompb, &errmsg);
				if(!isOk) {
					TLOG_DEBUG(errmsg);
				}
			}
		}
		break;
	case Command::EXITROOM:
		{
			auto ps = getLoggedPlayer(tcpConnPtr);
			if(ps) {
				std::string errmsg;
				if(ps->exitRoom(&errmsg)) {
					TLOG_DEBUG("Exit room success.")
				} else {
					TLOG_DEBUG("Exit room failed, " << errmsg);
				}
			}
		}
		break;
	
	default:
		TLOG_DEBUG("Invalid Command!");
		tcpConnPtr->send("Invalid Command!");
		break;
	}
}

std::shared_ptr<PlayerSession> GServer::getLoggedPlayer(const tinyserver::TcpConnectionPtr& tcpConnPtr)
{
	auto ps =  _ctrl.getPlayerSessionById(tcpConnPtr->id());
	if(ps == nullptr) {
		TLOG_DEBUG("用户未登陆");
	}
	return ps;
}