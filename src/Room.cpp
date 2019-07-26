#include "Room.h"
#include "PlayerSession.h"
#include "proto/playerpb.pb.h"

using namespace gserver;

Room::Room() :
	_roomPB(),
	_ownerPBPtr(new PlayerPB())
{	
}

Room::Room(const RoomPB& roomPB) :
	_roomPB(roomPB),
	_ownerPBPtr(new PlayerPB())
{	
}

Room::Room(RoomPB&& roomPB) :
	_roomPB(std::move(roomPB)),
	_ownerPBPtr(new PlayerPB())
{	
}

void Room::setOwner(const PlayerSession *player)
{
	assert(player != nullptr);
	auto playerPB = player->playerPB();
	_ownerPBPtr->set_id(playerPB.id());
	_ownerPBPtr->set_name(playerPB.name());
	_roomPB.set_allocated_owner(_ownerPBPtr.get());
}

bool Room::addPlayer(const PlayerSession *player, std::string *errmsg)
{
	auto playerPB = _roomPB.add_players();
	// playerPB->CopyFrom(player->playerPB());
	playerPB->set_id(player->playerPB().id());
	playerPB->set_name(player->playerPB().name());
	if(errmsg) {
		*errmsg = "";
	}
	return true;
}

int Room::playerCounter() const
{
	return _roomPB.players_size();
}

bool Room::removePlayer(const std::string& playerId, std::string *errmsg)
{
	auto players = _roomPB.mutable_players();
	for(auto p=players->cbegin(); p!=players->cend(); p++) {
		if(p->id() == playerId) {
			players->erase(p);
			return true;
		}
	}
	if(errmsg) {
		*errmsg = "玩家不在该房间";
	}
	return false;
}

bool Room::removePlayer(const PlayerPB *playerPB, std::string *errmsg)
{
	return removePlayer(playerPB->id(), errmsg);;
}

bool Room::hasPlayer(const std::string& playerId)
{
	for(auto p=_roomPB.players().cbegin(); p!=_roomPB.players().cend(); p++) {
		if(p->id() == playerId) {
			return true;
		}
	}
	return false;
}

bool Room::hasPlayer(const PlayerPB *playerPB)
{
	return hasPlayer(playerPB->id());
}

Room::~Room()
{
}