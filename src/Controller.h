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
		std::shared_ptr<PlayerSession> getPlayerSessionById(uint64_t id) const {
			try {
				return _playerSessionMap.at(id);
			} catch(const std::exception &e) {
				return nullptr;
			}
		}
		std::shared_ptr<Room> creatRoom();
		std::shared_ptr<Room> getRoomById(uint64_t id)  {
			try {
				return _roomMap.at(id);
			} catch(const std::exception &e) {
				return nullptr;
			}
		}
	private:
		std::mutex _mutex;
		uint64_t _roomIndex;
		std::map<uint64_t, std::shared_ptr<Room> > _roomMap;
		std::map<uint64_t, std::shared_ptr<PlayerSession> > _playerSessionMap;
	};
}

#endif // GSERVER_CONTROLLER_H