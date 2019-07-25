#include "PlayerSession.h"

#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

#include "Controller.h"
#include "Room.h"

using namespace gserver;
using namespace tinyserver;

PlayerSession::PlayerSession(Controller *ctrl, std::weak_ptr<tinyserver::TcpConnection> connection) :
	_ctrl(ctrl),
	_tcpConnection(connection)
{
	_playerPB.set_id(connection.lock()->id());
}

PlayerSession::~PlayerSession()
{
}

bool PlayerSession::createRoom(RoomPB *roomPB, std::string *errmsg)
{
	auto roomPtr = _ctrl->creatRoom();
	roomPB->set_id(roomPtr->roomPB().id());
	roomPB->set_allocated_owner(new PlayerPB(_playerPB));
	roomPtr->setRoomPB(roomPB);
	TLOG_DEBUG("Create room id: " << roomPtr->roomPB().id() << ", name: " << roomPtr->roomPB().name() << ", description: " << roomPtr->roomPB().description() << ", password: " << roomPtr->roomPB().password());
	bool isOk = joinRoom(roomPB, errmsg);
	return isOk;
}

bool PlayerSession::updateRoom(RoomPB *roomPB, std::string *errmsg)
{
	auto roomPtr = _ctrl->getRoomById(roomPB->id());
	if(!roomPtr) {
		TLOG_DEBUG("Can't find room by id: " << roomPB->id());
	} else {
		roomPtr->setRoomPB(roomPB);
		TLOG_DEBUG("Update room id: " << roomPtr->roomPB().id() << ", name: " << roomPtr->roomPB().name() << ", description: " << roomPtr->roomPB().description() << ", password: " << roomPtr->roomPB().password());
	}
	return true;
}

bool PlayerSession::joinRoom(RoomPB *roomPB, std::string *errmsg)
{
	if(_roomPtr.use_count() != 0) {		// 是否已在一个房间
		if(errmsg) {
			*errmsg = "需要先退出当前房间";
		}
		return false;
	}

	auto room = _ctrl->getRoomById(roomPB->id());
	if(!room) {
		if(errmsg) {
			*errmsg = "房间不存在";
		}
		return false;
	} else {
		_roomPtr = room;
		room->addPlayer(this);

		TLOG_DEBUG("Player id: " << _playerPB.id() << ", name:" << _playerPB.name() << " joined room id: " << _playerPB.id() << ", name: " << _playerPB.name());
		for(int i=0; i<room->playerCounter(); i++) {
			TLOG_DEBUG("===room player: " << room->roomPB().players(i).id());
		}
		return true;
	}
	return false;
}

bool PlayerSession::exitRoom(std::string *errmsg)
{
	if(_roomPtr.expired()) {
		if(errmsg) {
			*errmsg = "当前不在任何房间中";
		}
		return false;
	} else {
		_roomPtr.lock()->removePlayer(_playerPB.id());
		_roomPtr.reset();
		return true;
	}
	return false;
}