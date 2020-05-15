#include "Room.h"
#include "PlayerSession.h"
#include "proto/playerpb.pb.h"

using namespace gserver;
using namespace gserver::protobuf;

Room::Room() :
	_id(0),
	_name(""),
	_description(""),
	_password(""),
	_owner(),
	_players({}),
	_serverIP(""),
	_serverPort(0),
	_hasPassword(false),
	_chatRoomID(0),
	_gameRoomID(0)
{
}

Room::Room(const RoomPB& roomPB) :
	Room()
{
	setByRoomPB(roomPB);
}

Room::~Room()
{
}

void Room::setByRoomPB(const RoomPB& roomPB)
{
	_id = roomPB.id();
	_name = roomPB.name();
	_description = roomPB.description();
	_password = roomPB.password();
	_serverIP = roomPB.serverip();
	_serverPort = roomPB.serverport();
	_chatRoomID = roomPB.chatroomid();
	_gameRoomID = roomPB.gameroomid();
}

void Room::toRoomPB(RoomPB& roomPB)
{
	roomPB.set_id(_id);
	roomPB.set_name(_name);
	roomPB.set_description(_description);
	roomPB.set_password(_password);
	if (hasOwner()) {
		auto playerpb = new PlayerPB(owner()->playerPB());
		roomPB.set_allocated_owner(playerpb);
	}
	for (auto& pWPtr : players()) {
		auto pPtr = pWPtr.lock();
		if (pPtr) {
			auto playpb = roomPB.add_players();
			playpb->CopyFrom(pPtr->playerPB());
		}
	}
	roomPB.set_serverip(_serverIP);
	roomPB.set_serverport(_serverPort);
	if (_chatRoomID != 0) {
		roomPB.set_chatroomid(_chatRoomID);
	}
	if (_gameRoomID != 0) {
		roomPB.set_gameroomid(_gameRoomID);
	}
}

bool Room::addPlayer(const std::shared_ptr<PlayerSession>& player, std::string *errmsg)
{
	_players.push_back(player);
	if(errmsg) {
		*errmsg = "";
	}
	return true;
}

size_t Room::playerCounter() const
{
	return _players.size();
}

bool Room::removePlayer(const std::string& playerId, std::string *errmsg)
{
	for(auto p=_players.cbegin(); p!=_players.cend(); p++) {
		auto ps = p->lock();
		if(ps->playerPB().id() == playerId) {
			_players.erase(p); 
			return true;
		}
	}
	if(errmsg) {
		*errmsg = "玩家不在该房间";
	}
	return false;
}

bool Room::removePlayer(const PlayerPB& playerPB, std::string *errmsg)
{
	return removePlayer(playerPB.id(), errmsg);;
}

bool Room::hasPlayer(const std::string& playerId)
{
	for(auto p=_players.cbegin(); p!=_players.cend(); p++) {
		auto ps = p->lock();
		if(ps->playerPB().id() == playerId) {
			return true;
		}
	}
	return false;
}

bool Room::hasPlayer(const PlayerPB& playerPB)
{
	return hasPlayer(playerPB.id());
}