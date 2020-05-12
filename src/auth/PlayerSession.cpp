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
	if(roomPB->name() == "") {
		*errmsg = "房间名不能为空";
		return nullptr;
	}
	auto roomPtr = _userMgr->createRoom();
	roomPB->set_id(roomPtr->id());
	roomPtr->setByRoomPB(*roomPB);
	roomPtr->setOwner(shared_from_this());
	TLOG_DEBUG("Create room id: " << roomPtr->id() << ", name: " << roomPtr->name() << ", description: " << roomPtr->description() << ", password: " << roomPtr->password());
	auto joinRoomPtr = joinRoom(roomPB->id(), errmsg);
	return joinRoomPtr;
}

std::shared_ptr<Room> PlayerSession::updateRoom(RoomPB *roomPB, std::string *errmsg)
{
	auto roomPtr = _userMgr->getRoomById(roomPB->id());
	if(!roomPtr) {
		TLOG_DEBUG("Can't find room by id: " << roomPB->id());
	} else {
		roomPtr->setByRoomPB(*roomPB);
		TLOG_DEBUG("Update room id: " << roomPtr->id() << ", name: " << roomPtr->name() << ", description: " << roomPtr->description() << ", password: " << roomPtr->password());
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
		room->addPlayer(shared_from_this());

		TLOG_DEBUG("Player id: " << _playerPB.id() << ", name:" << _playerPB.name() << " joined room id: " << _playerPB.id() << ", name: " << _playerPB.name());
		for(auto& p : room->players()) {
			TLOG_DEBUG("===room player: " << p.lock()->playerPB().id());
		}
		return room;
	}
	return nullptr;
}

std::shared_ptr<Room> PlayerSession::exitRoom(int roomId, std::string *errmsg)
{
	for(auto it = _roomPtrList.begin(); it != _roomPtrList.end(); it++) {
		auto roomPtr = it->lock();
		if(roomPtr && (roomId == roomPtr->id())) {
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
		if(roomPtr && (roomId == roomPtr->id())) {
			return true;
		}
	}
	return false;
}