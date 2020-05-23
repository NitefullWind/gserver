#ifndef GSERVER_PLAYERSESSION_H
#define GSERVER_PLAYERSESSION_H

#include <list>
#include <memory>
#include <string>
#include <tinyserver/tcp/tcpConnection.h>
#include "common/uncopyable.h"
#include "proto/playerpb.pb.h"

namespace tinyserver
{
class TcpConnection;
}

namespace gserver
{
	class UserManager;
	class Room;
	namespace protobuf {
		class RoomPB;
		class PlayerPB;
	}

	class PlayerSession : private Uncopyable, public std::enable_shared_from_this<PlayerSession>
	{
	public:
		explicit PlayerSession(UserManager *userMgr, std::weak_ptr<tinyserver::TcpConnection> connection);
		virtual ~PlayerSession();

		inline void setPlayerPB(const protobuf::PlayerPB& playerPB) { _playerPB = playerPB; }
		inline void setPlayerPB(protobuf::PlayerPB&& playerPB) { _playerPB = std::move(playerPB); }
		inline const protobuf::PlayerPB& playerPB() const { return _playerPB; }
		inline protobuf::PlayerPB *mutablePlayerPB() { return &_playerPB; }

		std::shared_ptr<Room> createRoom(protobuf::RoomPB *roomPB, std::string *errmsg = nullptr);
		std::shared_ptr<Room> updateRoom(protobuf::RoomPB *roomPB, std::string *errmsg = nullptr);
		std::shared_ptr<Room> joinRoom(uint32_t roomId, std::string *errmsg = nullptr);
		std::shared_ptr<Room> exitRoom(uint32_t roomId, std::string *errmsg = nullptr);
		void exitAllRoom();
		bool isInRoom(uint32_t roomId);
		
		inline const std::list<std::weak_ptr<Room> >& RoomWeakPtrList() const { return _roomPtrList; }

		inline const std::weak_ptr<tinyserver::TcpConnection>& tcpConnectionWeakPtr() const { return _tcpConnection; }
	private:
		UserManager *_userMgr;
		protobuf::PlayerPB _playerPB;
		std::weak_ptr<tinyserver::TcpConnection> _tcpConnection;
		std::list<std::weak_ptr<Room> > _roomPtrList;
	};
	
}

#endif	//GSERVER_PLAYERSESSION_H