#include "Controller.h"

#include <assert.h>
#include <mutex>
#include <tinyserver/logger.h>

#include "PlayerSession.h"
#include "Room.h"


using namespace gserver;
using namespace tinyserver;

Controller::Controller() :
	_roomIndex(1)
{
}

Controller::~Controller()
{
}

bool Controller::login(std::shared_ptr<PlayerSession> playerSessionPtr, std::string *errmsg)
{
	auto id = playerSessionPtr->playerPB().id();
	if(hasPlayerSession(id)) {
		if(errmsg) {
			*errmsg = "用户已存在";
		}
		return false;
	}
	
	TLOG_DEBUG("Login player id: " << id << ", name: " << playerSessionPtr->playerPB().name() << ", password:" << playerSessionPtr->playerPB().password());

	{
		std::lock_guard<std::mutex> lk(_mutex);
		assert(_playerSessionMap.find(id) == _playerSessionMap.end());
		_playerSessionMap[id] = playerSessionPtr;
	}
	return true;
}

bool Controller::logout(const std::string& id, std::string *errmsg)
{
	auto ps = getPlayerSessionById(id);
	if(!ps) {
		if(errmsg) {
			*errmsg = "用户不存在";
		}
		return false;
	} else {
		ps->exitRoom();
	}
	{
		std::lock_guard<std::mutex> lk(_mutex);
		_playerSessionMap.erase(id);
	}
	return true;
}

bool Controller::hasPlayerSession(const std::string& id)
{
	std::lock_guard<std::mutex> lk(_mutex);
	return (_playerSessionMap.find(id) != _playerSessionMap.end());
}


std::shared_ptr<PlayerSession> Controller::getPlayerSessionById(const std::string& id)
{
	try {
		std::lock_guard<std::mutex> lk(_mutex);
		return _playerSessionMap.at(id);
	} catch(const std::exception &e) {
		return nullptr;
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

std::shared_ptr<Room> Controller::getRoomById(int id)  {
	try {
		std::lock_guard<std::mutex> lk(_mutex);
		return _roomMap.at(id);
	} catch(const std::exception &e) {
		return nullptr;
	}
}

std::map<int, std::shared_ptr<Room> > Controller::roomMap()
{
	std::lock_guard<std::mutex> lk(_mutex);
	return _roomMap;
}