#ifndef GSERVER_USERMANAGER_H
#define GSERVER_USERMANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <tinyserver/tcp/tcpConnection.h>

namespace gserver
{
	class PlayerSession;
	class Room;

	class UserManager
	{
	public:
		UserManager();
		~UserManager();

		bool login(std::shared_ptr<PlayerSession> playerSessionPtr, std::string *errmsg = nullptr);
		bool login(const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg = "", std::string *errmsg = nullptr);
		bool logout(const std::string& id, std::string *errmsg = nullptr);

		bool hasPlayerSession(const std::string& id);

		std::shared_ptr<PlayerSession> getPlayerSessionById(const std::string& id);
		std::shared_ptr<PlayerSession> getLoggedPlayer(const tinyserver::TcpConnectionPtr& tcpConnPtr, std::string *errmsg);
		std::shared_ptr<Room> creatRoom();
		std::shared_ptr<Room> getRoomById(int id);
		std::map<int, std::shared_ptr<Room> > roomMap();
	private:
		std::mutex _mutex;
		int _roomIndex;
		std::map<int, std::shared_ptr<Room> > _roomMap;
		std::map<std::string, std::shared_ptr<PlayerSession> > _playerSessionMap;
	};
}

#endif // GSERVER_USERMANAGER_H