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

std::shared_ptr<Room> PlayerSession::createRoom(RoomPB *roomPB, std::string *errmsg)
{
	auto roomPtr = _ctrl->creatRoom();
	roomPB->set_id(roomPtr->roomPB().id());
	roomPtr->setRoomPB(roomPB);
	roomPtr->setOwner(this);
	TLOG_DEBUG("Create room id: " << roomPtr->roomPB().id() << ", name: " << roomPtr->roomPB().name() << ", description: " << roomPtr->roomPB().description() << ", password: " << roomPtr->roomPB().password());
	auto joinRoomPtr = joinRoom(roomPB, errmsg);
	return joinRoomPtr;
}

std::shared_ptr<Room> PlayerSession::updateRoom(RoomPB *roomPB, std::string *errmsg)
{
	auto roomPtr = _ctrl->getRoomById(roomPB->id());
	if(!roomPtr) {
		TLOG_DEBUG("Can't find room by id: " << roomPB->id());
	} else {
		roomPtr->setRoomPB(roomPB);
		TLOG_DEBUG("Update room id: " << roomPtr->roomPB().id() << ", name: " << roomPtr->roomPB().name() << ", description: " << roomPtr->roomPB().description() << ", password: " << roomPtr->roomPB().password());
	}
	return roomPtr;
}

std::shared_ptr<Room> PlayerSession::joinRoom(RoomPB *roomPB, std::string *errmsg)
{
	if(_roomPtr.use_count() != 0) {		// 是否已在一个房间
		if(errmsg) {
			*errmsg = "需要先退出当前房间";
		}
		return nullptr;
	}

	auto room = _ctrl->getRoomById(roomPB->id());
	if(!room) {
		if(errmsg) {
			*errmsg = "房间不存在";
		}
		return nullptr;
	} else {
		_roomPtr = room;
		room->addPlayer(this);

		TLOG_DEBUG("Player id: " << _playerPB.id() << ", name:" << _playerPB.name() << " joined room id: " << _playerPB.id() << ", name: " << _playerPB.name());
		for(int i=0; i<room->playerCounter(); i++) {
			TLOG_DEBUG("===room player: " << room->roomPB().players(i).id());
		}
		return room;
	}
	return nullptr;
}

std::shared_ptr<Room> PlayerSession::exitRoom(std::string *errmsg)
{
	if(_roomPtr.expired()) {
		if(errmsg) {
			*errmsg = "当前不在任何房间中";
		}
		return nullptr;
	} else {
		auto room = _roomPtr.lock();
		_roomPtr.reset();
		room->removePlayer(_playerPB.id());
		return room;
	}
	return nullptr;
}