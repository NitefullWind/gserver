#ifndef GSERVER_ROOM_H
#define GSERVER_ROOM_H

#include <memory>
#include <string>
#include <set>
#include "common/uncopyable.h"
#include "proto/roompb.pb.h"

namespace gserver
{
	class UserManager;
	class PlayerSession;
	namespace protobuf {
		class PlayerPB;
	}

	class Room : private Uncopyable
	{
	public:
		friend  class UserManager;

		explicit Room();
		explicit Room(const protobuf::RoomPB& roomPB);
		~Room();

		uint32_t id() const { return _id; }
		void setId(uint32_t id) { _id = id; }

		const std::string& name() const { return _name; }
		void setName(const std::string& name) { _name = name; }

		const std::string& description() const { return _description; }
		void setDescription(const std::string& description) { _description = description; }

		const std::string& password() const { return _password; }
		void setPassword(const std::string& password) { _password = password; }

		std::shared_ptr<PlayerSession> owner() const { return _owner.lock(); }
		void setOwner(const std::shared_ptr<PlayerSession>& owner) { _owner = owner; }
		bool hasOwner() const { return _owner.use_count() > 0; }

		const std::vector<std::weak_ptr<PlayerSession>>& players() const { return _players; }
		std::shared_ptr<PlayerSession> players(size_t index) const { return _players[index].lock(); }

		const std::string& serverIP() const { return _serverIP; }
		void setServerIP(const std::string& serverIP) { _serverIP = serverIP; }

		uint32_t serverPort() const { return _serverPort; }
		void setServerPort(uint32_t serverPort) { _serverPort = serverPort; }

		bool hasPassword() const { return _hasPassword; }
		void setHasPassword(bool hasPassword) { _hasPassword = hasPassword; }

		uint32_t chatRoomID() const { return _chatRoomID; }
		void setChatRoomID(uint32_t chatRoomID) { _chatRoomID = chatRoomID; }

		uint32_t gameRoomID() const { return _gameRoomID; }
		void setGameRoomID(uint32_t gameRoomID) { _gameRoomID = gameRoomID; }

		/**
		 * @brief 使用RoomPB设置room信息
		 * 
		 * @param roomPB Room的Protobuf message对象
         * @note 这个函数只会设置基础类型的信息，对象的指针不会被设置，需要单独设置
		 */
		void setByRoomPB(const protobuf::RoomPB& roomPB);

		/**
		 * @brief 将房间信息保存到RoomPB中
		 * 
		 * @param roomPB Room的Protobuf message对象
		 */
		void toRoomPB(protobuf::RoomPB& roomPB);

		/**
		 * @brief 添加玩家指针
		 * 
		 * @param player 玩家指针
		 * @param errmsg 失败原因
		 * @return true 成功
		 * @return false 失败
		 */
		bool addPlayer(const std::shared_ptr<PlayerSession>& player, std::string *errmsg = nullptr);
		size_t playerCounter() const;

		bool removePlayer(const std::string& playerId, std::string *errmsg = nullptr);
		bool removePlayer(const protobuf::PlayerPB& playerPB, std::string *errmsg = nullptr);

		bool hasPlayer(const std::string& playerId);
		bool hasPlayer(const protobuf::PlayerPB& playerPB);
	private:
		uint32_t _id;
		std::string _name;
		std::string _description;
		std::string _password;
		std::weak_ptr<PlayerSession> _owner;					// 房间拥有者
		std::vector<std::weak_ptr<PlayerSession>> _players;		// 房间中的玩家
		std::string _serverIP;									// 房间所在服务器ip
		uint32_t _serverPort;									// 房间所在服务器端口
		bool _hasPassword;										// 是否有密码
		uint32_t _chatRoomID;									// 游戏房间的聊天房间id
		uint32_t _gameRoomID;									// 聊天房间的游戏房间id
	};
}

#endif	// GSERVER_ROOM_H