#include "PlayerSession.h"

#include <tinyserver/tcp/tcpConnection.h>

#include "Controller.h"
#include "Room.h"

using namespace gserver;

PlayerSession::PlayerSession(Controller *ctrl, std::weak_ptr<tinyserver::TcpConnection> connection) :
	_controller(ctrl),
	_sessionId(connection.lock()->id()),
	_tcpConnection(connection)
{
}

PlayerSession::~PlayerSession()
{
}

void PlayerSession::joinRoom(std::weak_ptr<Room> roomPtr)
{
	auto room = roomPtr.lock();
	if(room) {
		if(_roomPtr.use_count() != 0) {		// 退出已在的房间
			exitRoom();
		}
		_roomPtr = roomPtr;
		room->addPlayer(sessionId());
	}
}

void PlayerSession::exitRoom()
{
	auto room = _roomPtr.lock();
	if(room) {
		room->removePlayer(sessionId());
	}
}