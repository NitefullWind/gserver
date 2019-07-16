#include "PlayerSession.h"

#include <tinyserver/tcp/tcpConnection.h>

#include "Controller.h"
#include "Room.h"

using namespace gserver;

PlayerSession::PlayerSession(Controller *ctrl, std::shared_ptr<tinyserver::TcpConnection> connection) :
	_controller(ctrl),
	_tcpConnection(connection),
	_roomId(-1)
{
}

PlayerSession::~PlayerSession()
{
}

void PlayerSession::joinRoom(size_t roomId)
{
	auto room = _controller->getRoomById(roomId);
	if(room) {
		_roomId = room->id();
		room->addPlayer(sessionId());
	}
}

void PlayerSession::exitRoom()
{
	auto room = _controller->getRoomById(_roomId);
	if(room) {
		_roomId = -1;
		room->removePlayer(sessionId());
	}
}