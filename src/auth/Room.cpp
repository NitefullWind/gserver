#include "Room.h"
#include "PlayerSession.h"
#include "proto/playerpb.pb.h"

using namespace gserver;

Room::Room() :
	_id(0),
	_name(""),
	_description(""),
	_password(""),
	_ownerId(""),
	_players({}),
	_serverIP(""),
	_serverPort(0),
	_hasPassword(false),
	_customID(0),
	_relatedRoomID(0)
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
	if (roomPB.has_owner()) {
		_ownerId = roomPB.owner().id();
	}
	for (auto& p : roomPB.players()) {
		_players.push_back(p.id());
	}
	_serverIP = roomPB.serverip();
	_serverPort = roomPB.serverport();
	_customID = roomPB.customid();
	if (roomPB.has_relatedroom()) {
		_relatedRoomID = roomPB.relatedroom().id();
	}
}

void Room::toRoomPB(RoomPB& roomPB)
{
	roomPB.set_id(_id);
	roomPB.set_name(_name);
	roomPB.set_description(_description);
	roomPB.set_password(_password);
	if (_ownerId != "") {
		//TODO: set owner
		roomPB.set_allocated_owner(nullptr);
	}
	for (auto& p : roomPB.players()) {
		_players.push_back(p.id());
	}
	roomPB.set_serverip(_serverIP);
	roomPB.set_serverport(_serverPort);
	roomPB.set_customid(_customID);
	if (_relatedRoomID != 0) {
		//TODO: set relatedroom
		roomPB.set_allocated_relatedroom(nullptr);
	}
}

void Room::setOwner(const PlayerSession *player)
{
	assert(player != nullptr);
	_ownerId = player->playerPB().id();
}

bool Room::addPlayer(const PlayerSession *player, std::string *errmsg)
{
	_players.push_back(player->playerPB().id());
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
		if(*p == playerId) {
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
		if(*p == playerId) {
			return true;
		}
	}
	return false;
}

bool Room::hasPlayer(const PlayerPB& playerPB)
{
	return hasPlayer(playerPB.id());
}