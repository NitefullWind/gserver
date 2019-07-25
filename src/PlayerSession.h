#ifndef GSERVER_PLAYERSESSION_H
#define GSERVER_PLAYERSESSION_H

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
	class Controller;
	class Room;
	class RoomPB;

	class PlayerSession : private Uncopyable
	{
	public:
		explicit PlayerSession(Controller *ctrl, std::weak_ptr<tinyserver::TcpConnection> connection);
		~PlayerSession();

		void setPlayerPB(const PlayerPB& playerPB) { _playerPB = playerPB; }
		void setPlayerPB(PlayerPB&& playerPB) { _playerPB = std::move(playerPB); }
		const PlayerPB& playerPB() const { return _playerPB; }
		PlayerPB *mutablePlayerPB() { return &_playerPB; }

		bool createRoom(RoomPB *roomPB, std::string *errmsg = nullptr);
		bool updateRoom(RoomPB *roomPB, std::string *errmsg = nullptr);
		bool joinRoom(RoomPB *roomPB, std::string *errmsg = nullptr);
		bool exitRoom(std::string *errmsg = nullptr);
		
		const std::weak_ptr<Room>& RoomWeakPtr() const { return _roomPtr; }
	private:
		Controller *_ctrl;
		PlayerPB _playerPB;
		std::weak_ptr<tinyserver::TcpConnection> _tcpConnection;
		std::weak_ptr<Room> _roomPtr;
	};
	
}

#endif	//GSERVER_PLAYERSESSION_H