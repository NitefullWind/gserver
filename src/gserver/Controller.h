#ifndef GSERVER_CONTROLLER_H
#define GSERVER_CONTROLLER_H

#include <map>
#include <memory>
#include <mutex>

namespace gserver
{
	class PlayerSession;
	class Room;

	class Controller
	{
	public:
		Controller();
		~Controller();

		bool login(std::shared_ptr<PlayerSession> playerSessionPtr, std::string *errmsg = nullptr);
		bool logout(const std::string& id, std::string *errmsg = nullptr);

		bool hasPlayerSession(const std::string& id);

		std::shared_ptr<PlayerSession> getPlayerSessionById(const std::string& id);
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

#endif // GSERVER_CONTROLLER_H