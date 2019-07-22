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

		int id() const { return _id; }

		void setName(const std::string& name) { _name = name; }
		void setName(std::string&& name) { _name = std::move(name); }
		std::string name() const { return _name; }

		void setDescription(const std::string& description) { _description = description; }
		void setDescription(std::string&& description) { _description = std::move(description); }
		std::string description() const { return _description; }

		void setPassword(const std::string& password) { _password = password; }
		void setPassword(std::string&& password) { _password = std::move(password); }

		void addPlayer(std::string playerId) { _playerIdSet.insert(playerId); }
		size_t playerCounter() const { return _playerIdSet.size(); }

		void removePlayer(std::string playerId) { _playerIdSet.erase(playerId); }

		bool hasPlayer(std::string playerId) { return (_playerIdSet.find(playerId) != _playerIdSet.end()); }
	private:
		int _id;
		std::string _name;
		std::string _description;
		std::string _password;
		std::set<std::string> _playerIdSet;

		void setId(int id) { _id = id; }
	};
}

#endif	// GSERVER_ROOM_H