#ifndef GSERVER_USERMANAGER_H
#define GSERVER_USERMANAGER_H

#include <atomic>
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
		std::shared_ptr<Room> createRoom();
		std::shared_ptr<Room> getRoomById(uint32_t id);
		std::map<uint32_t, std::shared_ptr<Room> > roomMap();
	private:
		std::mutex _mutex;
		std::atomic_uint32_t _roomIndex;
		std::map<uint32_t, std::shared_ptr<Room> > _roomMap;
		std::map<std::string, std::shared_ptr<PlayerSession> > _playerSessionMap;
	};
}

#endif // GSERVER_USERMANAGER_H