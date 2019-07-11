#include <tinyserver/eventLoop.h>
#include <tinyserver/inetAddress.h>
#include <tinyserver/tcp/tcpServer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

#include "Controller.h"
#include "PlayerSession.h"
#include "Room.h"

using namespace tinyserver;
using namespace gserver;

enum Command
{
	CMD_JOINROOM = 100,
};

Controller gController;

void onNewClient(const TcpConnectionPtr &tcpConnptr)
{
	std::shared_ptr<PlayerSession> ps(new PlayerSession(tcpConnptr));
	gController.addPlayerSession(ps);
}

void onNewMessage(const TcpConnectionPtr &tcpConnPtr, Buffer *buffer)
{
	TLOG_DEBUG(buffer->read(1));
	// Command cmd = (Command)std::stoi(buffer->read(1));
	Command cmd = (Command)100;
	switch (cmd)
	{
	case CMD_JOINROOM:
		{
			// auto roomId = std::stoul(buffer->read(1));
			auto roomId = 0;
			auto room = gController.getRoomById(roomId);
			auto ps = gController.getPlayerSessionById(tcpConnPtr->index());
			if(room != nullptr && ps != nullptr) {
				room->addPlayer(ps->sessionId());
				TLOG_DEBUG("room players:" << room->playerCounter());
			}
		}
		break;
	
	default:
		TLOG_DEBUG("Invalid Command!");
		tcpConnPtr->send("Invalid Command!");
		break;
	}
}

int main(int argc, char **argv)
{
	Logger::SetLevel(Logger::Debug);
	
	auto room = gController.creatRoom();
	room->setName("testRoom");
	room->setDescription("this is a test room!");
	room->setPassword("111111");

	EventLoop loop;
	TcpServer server(&loop, InetAddress(8086));
	server.setIOThreadNum(2);
	server.setConnectionCallback(onNewClient);
	server.setMessageCallback(onNewMessage);
	server.start();
	loop.loop();
	return 0;
}