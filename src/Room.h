#ifndef GSERVER_ROOM_H
#define GSERVER_ROOM_H

#include <memory>
#include <string>
#include <set>
#include "common/uncopyable.h"

namespace gserver
{
	class Controller;
	class PlayerSession;

	class Room : private Uncopyable
	{
	public:
		friend  class Controller;

		explicit Room(const std::string &name);
		explicit Room(std::string &&name="");
		~Room();

		uint64_t id() const { return _id; }

		void setName(const std::string& name) { _name = name; }
		void setName(std::string&& name) { _name = std::move(name); }
		std::string name() const { return _name; }

		void setDescription(const std::string& description) { _description = description; }
		void setDescription(std::string&& description) { _description = std::move(description); }
		std::string description() const { return _description; }

		void setPassword(const std::string& password) { _password = password; }
		void setPassword(std::string&& password) { _password = std::move(password); }

		void addPlayer(uint64_t playerId) { _playerIdSet.insert(playerId); }
		size_t playerCounter() const { return _playerIdSet.size(); }

		void removePlayer(uint64_t playerId) { _playerIdSet.erase(playerId); }

		bool hasPlayer(uint64_t playerId) { return (_playerIdSet.find(playerId) != _playerIdSet.end()); }
	private:
		uint64_t _id;
		std::string _name;
		std::string _description;
		std::string _password;
		std::set<uint64_t> _playerIdSet;

		void setId(uint64_t id) { _id = id; }
	};
}

#endif	// GSERVER_ROOM_H