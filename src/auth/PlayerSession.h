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
	class RoomPB;

	class PlayerSession : private Uncopyable, public std::enable_shared_from_this<PlayerSession>
	{
	public:
		explicit PlayerSession(UserManager *userMgr, std::weak_ptr<tinyserver::TcpConnection> connection);
		~PlayerSession();

		void setPlayerPB(const PlayerPB& playerPB) { _playerPB = playerPB; }
		void setPlayerPB(PlayerPB&& playerPB) { _playerPB = std::move(playerPB); }
		const PlayerPB& playerPB() const { return _playerPB; }
		PlayerPB *mutablePlayerPB() { return &_playerPB; }

		std::shared_ptr<Room> createRoom(RoomPB *roomPB, std::string *errmsg = nullptr);
		std::shared_ptr<Room> updateRoom(RoomPB *roomPB, std::string *errmsg = nullptr);
		std::shared_ptr<Room> joinRoom(int roomId, std::string *errmsg = nullptr);
		std::shared_ptr<Room> exitRoom(int roomId, std::string *errmsg = nullptr);
		void exitAllRoom();
		bool isInRoom(int roomId);
		
		const std::list<std::weak_ptr<Room> >& RoomWeakPtrList() const { return _roomPtrList; }

		const std::weak_ptr<tinyserver::TcpConnection> tcpConnectionWeakPtr() const { return _tcpConnection; }
	private:
		UserManager *_userMgr;
		PlayerPB _playerPB;
		std::weak_ptr<tinyserver::TcpConnection> _tcpConnection;
		std::list<std::weak_ptr<Room> > _roomPtrList;
	};
	
}

#endif	//GSERVER_PLAYERSESSION_H