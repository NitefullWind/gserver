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

	class PlayerSession : private Uncopyable
	{
	public:
		explicit PlayerSession(Controller *ctrl, std::shared_ptr<tinyserver::TcpConnection> connection);
		~PlayerSession();

		void setName(const std::string& name) { _name = name; }
		void setName(std::string&& name) { _name = std::move(name); }
		std::string name() const { return _name; }

		uint64_t sessionId() const { return _tcpConnection->index(); }

		const std::shared_ptr<tinyserver::TcpConnection>& tcpConnction() const { return _tcpConnection; }

		void joinRoom(size_t roomId);
		void exitRoom();
		size_t roomId() const { return _roomId; }
	private:
		Controller *_controller;
		std::string _name;
		std::shared_ptr<tinyserver::TcpConnection> _tcpConnection;
		size_t _roomId;
	};
	
}

#endif	//GSERVER_PLAYERSESSION_H