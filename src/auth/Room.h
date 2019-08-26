#ifndef GSERVER_ROOM_H
#define GSERVER_ROOM_H

#include <memory>
#include <string>
#include <set>
#include "common/uncopyable.h"
#include "proto/roompb.pb.h"

namespace gserver
{
	class UserManager;
	class PlayerSession;
	class PlayerPB;

	class Room : private Uncopyable
	{
	public:
		friend  class UserManager;

		explicit Room();
		explicit Room(const RoomPB& roomPB);
		explicit Room(RoomPB&& roomPB);
		~Room();

		void setRoomPB(const RoomPB& roomPB) { _roomPB = roomPB; }
		void setRoomPB(RoomPB&& roomPB) { _roomPB = std::move(roomPB); }
		void setRoomPB(RoomPB *roomPB) { _roomPB = *roomPB; }
		const RoomPB& roomPB() const { return _roomPB; }
		RoomPB *mutableRoomPB() { return &_roomPB; }

		void setOwner(const PlayerSession *player);

		bool addPlayer(const PlayerSession *player, std::string *errmsg = nullptr);
		int playerCounter() const;
		const google::protobuf::RepeatedPtrField<gserver::PlayerPB>& allPlayerPB() const;

		bool removePlayer(const std::string& playerId, std::string *errmsg = nullptr);
		bool removePlayer(const PlayerPB *playerPB, std::string *errmsg = nullptr);

		bool hasPlayer(const std::string& playerId);
		bool hasPlayer(const PlayerPB *playerPB);
	private:
		RoomPB _roomPB;
		std::shared_ptr<PlayerPB> _ownerPBPtr;

		void setId(int id) { _roomPB.set_id(id); }
	};
}

#endif	// GSERVER_ROOM_H