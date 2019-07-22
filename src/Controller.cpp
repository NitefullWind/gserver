#include "Controller.h"

#include <assert.h>
#include <mutex>

#include "PlayerSession.h"
#include "Room.h"


using namespace gserver;

Controller::Controller() :
	_roomIndex(0)
{
}

Controller::~Controller()
{
}

void Controller::addPlayerSession(std::shared_ptr<PlayerSession> playerSessionPtr)
{
	auto id = playerSessionPtr->sessionId();
	{
		std::lock_guard<std::mutex> lk(_mutex);
		assert(_playerSessionMap.find(id) == _playerSessionMap.end());
		_playerSessionMap[id] = playerSessionPtr;
	}
}

std::shared_ptr<Room> Controller::creatRoom()
{
	std::shared_ptr<Room> roomPtr(new Room());
	int id = 0;
	{
		std::lock_guard<std::mutex> lk(_mutex);
		id = _roomIndex++;
		assert(_roomMap.find(id) == _roomMap.end());
		_roomMap[id] = roomPtr;
	}
	roomPtr->setId(id);
	return roomPtr;
}