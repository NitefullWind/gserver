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

		void addPlayerSession(std::shared_ptr<PlayerSession> playerSessionPtr);
		void removePlayerSession(std::string id) { _playerSessionMap.erase(id); }
		std::shared_ptr<PlayerSession> getPlayerSessionById(std::string id) const {
			try {
				return _playerSessionMap.at(id);
			} catch(const std::exception &e) {
				return nullptr;
			}
		}
		std::shared_ptr<Room> creatRoom();
		std::shared_ptr<Room> getRoomById(int id)  {
			try {
				return _roomMap.at(id);
			} catch(const std::exception &e) {
				return nullptr;
			}
		}
	private:
		std::mutex _mutex;
		int _roomIndex;
		std::map<int, std::shared_ptr<Room> > _roomMap;
		std::map<std::string, std::shared_ptr<PlayerSession> > _playerSessionMap;
	};
}

#endif // GSERVER_CONTROLLER_H