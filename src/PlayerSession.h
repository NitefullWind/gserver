#ifndef GSERVER_PLAYERSESSION_H
#define GSERVER_PLAYERSESSION_H

#include <memory>
#include <string>
#include <tinyserver/tcp/tcpConnection.h>
#include "common/uncopyable.h"

namespace tinyserver
{
class TcpConnection;
}

namespace gserver
{
	class Controller;
	class Room;

	class PlayerSession : private Uncopyable
	{
	public:
		explicit PlayerSession(Controller *ctrl, std::weak_ptr<tinyserver::TcpConnection> connection);
		~PlayerSession();

		void setName(const std::string& name) { _name = name; }
		void setName(std::string&& name) { _name = std::move(name); }
		std::string name() const { return _name; }

		std::string sessionId() const { return _sessionId; }

		void joinRoom(std::weak_ptr<Room> roomPtr);
		void exitRoom();
		// size_t roomId() const { return _roomId; }
	private:
		Controller *_controller;
		std::string _sessionId;
		std::string _name;
		std::weak_ptr<tinyserver::TcpConnection> _tcpConnection;
		std::weak_ptr<Room> _roomPtr;
	};
	
}

#endif	//GSERVER_PLAYERSESSION_H