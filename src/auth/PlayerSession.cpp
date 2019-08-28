#include "PlayerSession.h"

#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

#include "UserManager.h"
#include "Room.h"

using namespace gserver;
using namespace tinyserver;

PlayerSession::PlayerSession(UserManager *userMgr, std::weak_ptr<tinyserver::TcpConnection> connection) :
	_userMgr(userMgr),
	_tcpConnection(connection)
{
	_playerPB.set_id(connection.lock()->id());
}

PlayerSession::~PlayerSession()
{
}

std::shared_ptr<Room> PlayerSession::createRoom(RoomPB *roomPB, std::string *errmsg)
{
	auto roomPtr = _userMgr->createRoom();
	roomPB->set_id(roomPtr->roomPB().id());
	roomPtr->setRoomPB(roomPB);
	roomPtr->setOwner(this);
	TLOG_DEBUG("Create room id: " << roomPtr->roomPB().id() << ", name: " << roomPtr->roomPB().name() << ", description: " << roomPtr->roomPB().description() << ", password: " << roomPtr->roomPB().password());
	auto joinRoomPtr = joinRoom(roomPB->id(), errmsg);
	return joinRoomPtr;
}

std::shared_ptr<Room> PlayerSession::updateRoom(RoomPB *roomPB, std::string *errmsg)
{
	auto roomPtr = _userMgr->getRoomById(roomPB->id());
	if(!roomPtr) {
		TLOG_DEBUG("Can't find room by id: " << roomPB->id());
	} else {
		roomPtr->setRoomPB(roomPB);
		TLOG_DEBUG("Update room id: " << roomPtr->roomPB().id() << ", name: " << roomPtr->roomPB().name() << ", description: " << roomPtr->roomPB().description() << ", password: " << roomPtr->roomPB().password());
	}
	return roomPtr;
}

std::shared_ptr<Room> PlayerSession::joinRoom(int roomId, std::string *errmsg)
{
	if(roomId <= 0) {
		if(errmsg) {
			*errmsg = "房间号不正确";
		}
		return nullptr;
	}
	if(isInRoom(roomId)) {		// 是否已在此房间
		if(errmsg) {
			*errmsg = "已在该房间中";
		}
		return nullptr;
	}

	auto room = _userMgr->getRoomById(roomId);
	if(!room) {
		if(errmsg) {
			*errmsg = "房间不存在";
		}
		return nullptr;
	} else {
		_roomPtrList.push_back(room);
		room->addPlayer(this);

		TLOG_DEBUG("Player id: " << _playerPB.id() << ", name:" << _playerPB.name() << " joined room id: " << _playerPB.id() << ", name: " << _playerPB.name());
		for(int i=0; i<room->playerCounter(); i++) {
			TLOG_DEBUG("===room player: " << room->roomPB().players(i).id());
		}
		return room;
	}
	return nullptr;
}

std::shared_ptr<Room> PlayerSession::exitRoom(int roomId, std::string *errmsg)
{
	for(auto it = _roomPtrList.begin(); it != _roomPtrList.end(); it++) {
		auto roomPtr = it->lock();
		if(roomPtr && (roomId == roomPtr->roomPB().id())) {
			_roomPtrList.erase(it);
			roomPtr->removePlayer(_playerPB.id());
			return roomPtr;
		}
	}
	if(errmsg) {
		*errmsg = "不在该房间中";
	}
	return nullptr;
}

void PlayerSession::exitAllRoom()
{
	for(auto roomWeakPtr : _roomPtrList) {
		auto roomPtr = roomWeakPtr.lock();
		if(roomPtr) {
			roomPtr->removePlayer(_playerPB.id());
		}
	}
	_roomPtrList.clear();
}

bool PlayerSession::isInRoom(int roomId)
{
	if(roomId <= 0) {
		return false;
	}
	for(auto roomWeakPtr : _roomPtrList) {
		auto roomPtr = roomWeakPtr.lock();
		if(roomPtr && (roomId == roomPtr->roomPB().id())) {
			return true;
		}
	}
	return false;
}