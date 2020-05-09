#include "UserManager.h"

#include <assert.h>
#include <mutex>
#include <tinyserver/logger.h>

#include "PlayerSession.h"
#include "Room.h"


using namespace gserver;
using namespace tinyserver;

UserManager::UserManager() :
	_roomIndex(1)
{
}

UserManager::~UserManager()
{
}

bool UserManager::login(std::shared_ptr<PlayerSession> playerSessionPtr, std::string *errmsg)
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

bool UserManager::login(const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg, std::string *errmsg)
{
	std::shared_ptr<PlayerSession> ps(new PlayerSession(this, tcpConnPtr));
	PlayerPB playerpb;
	playerpb.ParseFromString(reqMsg);
	playerpb.set_id(tcpConnPtr->id());	//将TcpConnection的id设为用户id
	ps->setPlayerPB(std::move(playerpb));
	return login(ps, errmsg);
}

bool UserManager::logout(const std::string& id, std::string *errmsg)
{
	auto ps = getPlayerSessionById(id);
	if(!ps) {
		if(errmsg) {
			*errmsg = "用户不存在";
		}
		return false;
	} else {
		ps->exitAllRoom();
	}
	{
		std::lock_guard<std::mutex> lk(_mutex);
		_playerSessionMap.erase(id);
	}
	return true;
}

bool UserManager::hasPlayerSession(const std::string& id)
{
	std::lock_guard<std::mutex> lk(_mutex);
	return (_playerSessionMap.find(id) != _playerSessionMap.end());
}


std::shared_ptr<PlayerSession> UserManager::getPlayerSessionById(const std::string& id)
{
	try {
		std::lock_guard<std::mutex> lk(_mutex);
		return _playerSessionMap.at(id);
	} catch(const std::exception &e) {
		return nullptr;
	}
}

std::shared_ptr<PlayerSession> UserManager::getLoggedPlayer(const tinyserver::TcpConnectionPtr& tcpConnPtr, std::string *errmsg)
{
	auto ps = getPlayerSessionById(tcpConnPtr->id());
	if(ps == nullptr) {
		if(errmsg) {
			*errmsg = "用户未登陆";
		}
		TLOG_DEBUG(*errmsg);
	}
	return ps;
}

std::shared_ptr<Room> UserManager::createRoom()
{
	auto roomPtr = std::make_shared<Room>();
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

std::shared_ptr<Room> UserManager::getRoomById(int id)  {
	try {
		std::lock_guard<std::mutex> lk(_mutex);
		return _roomMap.at(id);
	} catch(const std::exception &e) {
		return nullptr;
	}
}

std::map<int, std::shared_ptr<Room> > UserManager::roomMap()
{
	std::lock_guard<std::mutex> lk(_mutex);
	return _roomMap;
}
