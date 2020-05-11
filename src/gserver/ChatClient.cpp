#include "ChatClient.h"

#include <assert.h>
#include <memory>
#include <tinyserver/buffer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

#include "auth/PlayerSession.h"
#include "auth/Room.h"
#include "auth/UserManager.h"
#include "common/parseMessageHeader.h"
#include "proto/roompb.pb.h"

using namespace gserver;
using namespace tinyserver;

ChatClient::ChatClient(EventLoop *loop, UserManager *userMgr) :
    _tcpClient(loop, InetAddress("127.0.0.1", 8087)),
    _started(false),
    _isLogin(false),
    _userMgr(userMgr)
{
}

ChatClient::~ChatClient()
{
}

void ChatClient::start()
{
	assert(_started == false);
	_started = true;

	_tcpClient.setRetry(true);
	_tcpClient.setConnectionCallback([](const TcpConnectionPtr& connPtr) {
		TLOG_INFO("连接到聊天服务器:" << connPtr->peerAddress().toHostPort());
		PlayerPB admin_gserver;
		admin_gserver.set_name("Admin GServer");
		sendMessageToConnection(connPtr, Command::LOGIN, 0, admin_gserver.SerializePartialAsString());
	});

    // 返回数据处理
	_tcpClient.setMessageCallback([this](const TcpConnectionPtr& connPtr, Buffer *buffer) {
		MessageHeader header = {0, Command::INVILID, 0, 0, RspCode::SUCCESS, 0};
		Buffer rspBuffer;
		std::string errmsg;
		if(parseMessageHeader(buffer, header, &errmsg)) {
			this->processResponse(header, connPtr, buffer->read(header.datalen));
		} else {
			TLOG_INFO(errmsg);
		}
	});

	_tcpClient.setDisconnectionCallback([](const TcpConnectionPtr& connPtr) {
		TLOG_INFO("已断开聊天服务器:" << connPtr->peerAddress().toHostPort());
	});
	_tcpClient.connect();
}

bool ChatClient::createRoom(RoomPB *roompb, std::string *errmsg)
{
    assert(roompb);

    if(!_isLogin) {
        *errmsg = "主服务器未连接到聊天服务器";
        return false;
    }
    sendMessageToConnection(_tcpClient.tcpConnection(), Command::CREATEROOM, 0, roompb->SerializePartialAsString());
    return true;
}

void ChatClient::processResponse(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& rspMsg)
{
	std::string errmsg;
	switch (header.cmd)
	{
	case Command::LOGIN:
		{
            if(header.rspcode == RspCode::SUCCESS) {
                TLOG_DEBUG("ChatClient登录成功");
                _isLogin = true;
                // 创建全局聊天房间
                RoomPB roompb;
                roompb.set_name("GServer Room");
                this->createRoom(&roompb);
            } else {
                TLOG_ERROR("ChatClient登录失败：" << rspMsg);
            }
        }
        break;
    case Command::CREATEROOM:
        {
            if(header.rspcode == RspCode::SUCCESS) {
                TLOG_DEBUG("ChatClient创建房间成功");
                RoomPB roompb;
				roompb.ParseFromString(rspMsg);
                auto customId = roompb.customid(); // 游戏房间id
                if(customId > 0) {
                    auto gameRoomPtr = _userMgr->getRoomById(customId);
                    if(gameRoomPtr) {
                        TLOG_DEBUG(gameRoomPtr->name() << " 的聊天房间创建成功！")
                        gameRoomPtr->setRelatedRoomID(customId);
                        auto player = _userMgr->getPlayerSessionById(gameRoomPtr->ownerId());
                        auto psConnection = player->tcpConnectionWeakPtr().lock();
                        if(psConnection) {
                            RoomPB retpb;
                            gameRoomPtr->toRoomPB(retpb);
                            sendMessageToConnection(psConnection, Command::UPDATEROOM, 0, retpb.SerializePartialAsString());
                        } else {
                            TLOG_INFO("房主已不在线");
                        }
                    } else {
                        TLOG_INFO(roompb.customid() << " 所对应的游戏房间不存在！")
                        //!TODO 删除创建的房间
                    }
                }
            } else {
                TLOG_ERROR("ChatClient创建房间失败：" << rspMsg);
            }
        }
        break;
	default:
		{
            TLOG_DEBUG("ChatClient未处理的CMD：" << (int)header.cmd);
        }
		break;
    }
}