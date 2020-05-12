#include "ChatServer.h"

#include <tinyserver/buffer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

#include "auth/PlayerSession.h"
#include "auth/Room.h"
#include "parseMessageHeader.h"

using namespace gserver;
using namespace tinyserver;

ChatServer::ChatServer()
{
}

ChatServer::~ChatServer()
{
}

void ChatServer::init()
{
	// createDefaultChatRoom();
}

void ChatServer::onNewConnection(const tinyserver::TcpConnectionPtr& tcpConnPtr)
{

}

void ChatServer::onDisconnection(const tinyserver::TcpConnectionPtr& tcpConnPtr)
{
	_userMgr.logout(tcpConnPtr->id());
}

void ChatServer::processRequest(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& reqMsg, Buffer *rspBuffer)
{
	std::string errmsg;
	switch (header.cmd)
	{
		case Command::SENDMSG:
		{
			auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
			if(ps) {
				MessagePB msgPB;
				msgPB.ParseFromString(reqMsg);
				switch (msgPB.messagetype())
				{
				case MessagePB::MT_PRIVATE_CHAT:
					{
						std::string receiverId = msgPB.receiverid();
						if(receiverId != "") {
							msgPB.set_allocated_sender(ps->mutablePlayerPB());
							bool isOk = sendMsgToUser(receiverId, msgPB, &errmsg);
							msgPB.release_sender();
							if(!isOk) {
								header.rspcode = RspCode::ERROR;
								rspBuffer->append(errmsg);
							}
						} else {
							header.rspcode = RspCode::ERROR;
							rspBuffer->append("用户id错误");
						}
					}
					break;
				case MessagePB::MT_GROUP_CHAT:
					{
						int groupId = 0;
						try {
							groupId = std::stoi(msgPB.receiverid());
						} catch (std::invalid_argument) {
							TLOG_DEBUG("Invalid group id: " << msgPB.receiverid());
						} catch (std::out_of_range) {
							TLOG_DEBUG("Group id: " << msgPB.receiverid() << " out of range");
						}
						if(groupId > 0) {
							std::shared_ptr<Room> roomPtr = _userMgr.getRoomById(groupId);
							if(roomPtr) {
								if (ps->isInRoom(roomPtr->id())) {
									msgPB.set_allocated_sender(ps->mutablePlayerPB());
									bool isOk = sendMsgToGroup(roomPtr->id(), msgPB, &errmsg);
									msgPB.release_sender(); // 释放指针的控制权，避免在MessagePB的析构函数中被析构
									if(!isOk) {
										header.rspcode = RspCode::ERROR;
										rspBuffer->append(errmsg);
									}
								} else {
									header.rspcode = RspCode::ERROR;
									rspBuffer->append("用户不在该房间中");
								}
							} else {
								TLOG_DEBUG("Don't find room: " << msgPB.receiverid());
								header.rspcode = RspCode::ERROR;
								rspBuffer->append("房间不存在");
							}
						} else {
							header.rspcode = RspCode::ERROR;
							rspBuffer->append("房间号错误");
						}
					}
					break;
				default:
					{
						header.rspcode = RspCode::ERROR;
						rspBuffer->append("Invalid Message Type");
					}
					break;
				}
			} else {
				header.rspcode = RspCode::ERROR;
				rspBuffer->append(errmsg);
			}
			break;
		}
		case Command::LOGIN:
			{
				if(!_userMgr.login(tcpConnPtr, reqMsg, &errmsg)) {
					TLOG_DEBUG(errmsg);
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				} else {
					// 加入全服聊天室
					auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
					ps->joinRoom(1, &errmsg);
					// 返回房间列表
					RoomPBList allRoomPB;
					for(auto room : _userMgr.roomMap()) {
						auto roompb = allRoomPB.add_roompb();
						room.second->toRoomPB(*roompb);
						roompb->set_password("");
					}
					rspBuffer->append(allRoomPB.SerializePartialAsString());
				}
			}
			break;
		case Command::LOGOUT:
			{
				if(!_userMgr.logout(tcpConnPtr->id(), &errmsg)) {
					TLOG_DEBUG(errmsg);
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				}
			}
			break;
		case Command::CREATEROOM:
			{
				auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
				if(ps) {
					RoomPB roompb;
					roompb.ParseFromString(reqMsg);
					auto roomPtr = ps->createRoom(&roompb, &errmsg);
					if(!roomPtr) {
						TLOG_DEBUG(errmsg);
						header.rspcode = RspCode::ERROR;
						rspBuffer->append(errmsg);
					} else {	// 返回房间信息
						RoomPB retpb;
						roomPtr->toRoomPB(retpb);
						retpb.set_password("");
						rspBuffer->append(retpb.SerializePartialAsString());
					}
				} else {
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				}
			}
			break;
		case Command::UPDATEROOM:
			{
				auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
				if(ps) {
					RoomPB roompb;
					roompb.ParseFromString(reqMsg);
					std::string errmsg;
					auto roomPtr = ps->updateRoom(&roompb, &errmsg);
					if(!roomPtr) {
						TLOG_DEBUG(errmsg);
						header.rspcode = RspCode::ERROR;
						rspBuffer->append(errmsg);
					}  else {	// 返回房间信息
						RoomPB retpb;
						roomPtr->toRoomPB(retpb);
						retpb.set_password("");
						rspBuffer->append(retpb.SerializePartialAsString());
					}
				} else {
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				}
			}
			break;
		case Command::JOINROOM:
			{
				auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
				if(ps) {
					RoomPB roompb;
					roompb.ParseFromString(reqMsg);
					auto roomPtr = ps->joinRoom(roompb.id(), &errmsg);
					if(!roomPtr) {
						TLOG_DEBUG(errmsg);
						header.rspcode = RspCode::ERROR;
						rspBuffer->append(errmsg);
					} else {	// 返回房间信息
						RoomPB retpb;
						roomPtr->toRoomPB(retpb);
						retpb.set_password("");
						rspBuffer->append(retpb.SerializePartialAsString());
					}
				} else {
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				}
			}
			break;
		case Command::EXITROOM:
			{
				auto ps = _userMgr.getLoggedPlayer(tcpConnPtr, &errmsg);
				if(ps) {
					RoomPB roompb;
					roompb.ParseFromString(reqMsg);
					if(ps->exitRoom(roompb.id(), &errmsg)) {
						TLOG_DEBUG("Exit room success.")
					} else {
						TLOG_DEBUG("Exit room failed, " << errmsg);
						header.rspcode = RspCode::ERROR;
						rspBuffer->append(errmsg);
					}
				} else {
					header.rspcode = RspCode::ERROR;
					rspBuffer->append(errmsg);
				}
			}
			break;
		
		default:
			TLOG_DEBUG("Invalid Command!");
			header.rspcode = RspCode::ERROR;
			rspBuffer->append("Invalid Command!");
			break;
	}
}


bool ChatServer::sendMsgToGroup(const RoomPB& roomPB, const MessagePB& msgPB, std::string *errmsg)
{
	std::string senderId = "";
	if(msgPB.has_sender()) {
		senderId = msgPB.sender().id();
	}
	return sendMsgToGroup(roomPB.id(), senderId, msgPB.SerializePartialAsString(), errmsg);
}

bool ChatServer::sendMsgToGroup(int32_t groupId, const MessagePB& msgPB, std::string *errmsg)
{
	std::string senderId = "";
	if(msgPB.has_sender()) {
		senderId = msgPB.sender().id();
	}
	return sendMsgToGroup(groupId, senderId, msgPB.SerializePartialAsString(), errmsg);
}

bool ChatServer::sendMsgToGroup(int32_t groupId, const std::string& senderId, const std::string& msgPBStr, std::string *errmsg)
{
	auto room = _userMgr.getRoomById(groupId);
	if(room) {
		for(auto& p : room->players()) {
			auto ps = p.lock();
			if (!ps) {
				if(errmsg) {
					*errmsg = "用户不存在";
				}
				return false;
			}
			if (senderId != ps->playerPB().id()) {
				sendMsgToUser(senderId, msgPBStr);
			}
		}
		return true;
	} else {
		if(errmsg) {
			*errmsg = "房间不存在";
		}
	}
	return false;
}

bool ChatServer::sendMsgToUser(const PlayerPB& playerPB, const MessagePB& msgPB, std::string *errmsg)
{
	return sendMsgToUser(playerPB.id(), msgPB, errmsg);
}

bool ChatServer::sendMsgToUser(const std::string& userId, const MessagePB& msgPB, std::string *errmsg)
{
	const std::string& msgPBStr = msgPB.SerializePartialAsString();
	return sendMsgToUser(userId, msgPBStr, errmsg);
}

/**
 * @brief 向用户发送消息
 * 
 * @param userId 用户ID
 * @param msgPBStr google protobuf 格式的消息字符串
 * @param errmsg 出错信息
 * @return true 发送成功
 * @return false 发送失败
 */
bool ChatServer::sendMsgToUser(const std::string& userId, const std::string& msgPBStr, std::string *errmsg)
{
	auto ps = _userMgr.getPlayerSessionById(userId);
	if(ps) {
		auto psTcpConnection = ps->tcpConnectionWeakPtr().lock();
		
		if(psTcpConnection) {
			MessageHeader header = {MessageHeaderFlag, Command::RECVMSG, 0, static_cast<uint32_t>(msgPBStr.length()), RspCode::SUCCESS, MessageHeaderVersion};
			Buffer rspBuffer;
			writeHeaderToBuffer(&rspBuffer, header);
			rspBuffer.append(msgPBStr);
			auto sendStr = rspBuffer.readAll();
			TLOG_TRACE("Send to user[" << userId << "]: " << sendStr);
			psTcpConnection->send(sendStr);
			return true;
		} else {
			if(errmsg) {
				*errmsg = "用户不在线";
			}
		}
	} else {
		if(errmsg) {
			*errmsg = "用户不在线";
		}
	}
	return false;
}

void ChatServer::createDefaultChatRoom()
{
	auto roomPtr = _userMgr.createRoom();
	roomPtr->setName("全服");
}