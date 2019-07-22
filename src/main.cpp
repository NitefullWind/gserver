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
	CMD_INVILID,
	CMD_JOINROOM = 100,
	CMD_EXITROOM = 101,
};

typedef struct
{
	uint16_t flag;
	uint16_t cmd;
	uint16_t datalen;
	
} RequestHeader;

#define RequestHeaderFlag 0x4753

Controller gController;

void onNewClient(const TcpConnectionPtr& tcpConnPtr)
{
	TLOG_DEBUG("Connection: " << tcpConnPtr->id());
	std::shared_ptr<PlayerSession> ps(new PlayerSession(&gController, tcpConnPtr));
	gController.addPlayerSession(ps);
}

void onDisconnection(const TcpConnectionPtr& tcpConnPtr)
{
	TLOG_DEBUG("Disconnection: " << tcpConnPtr->id());
	auto ps = gController.getPlayerSessionById(tcpConnPtr->id());
	if(ps) {
		ps->exitRoom();
		gController.removePlayerSession(ps->sessionId());
	}
}

void onNewMessage(const TcpConnectionPtr &tcpConnPtr, Buffer *buffer)
{
	if(buffer->readableBytes() < sizeof(RequestHeader)) {
		TLOG_DEBUG("Readable bytes less than " << sizeof(RequestHeader));
	} else {
		uint16_t checkFlag = buffer->readInt16();
		if(checkFlag != RequestHeaderFlag) {
			TLOG_DEBUG("Invalid request header flag:" << checkFlag);
			buffer->retrieve(sizeof(RequestHeader));
		} else {
			RequestHeader header = {0, CMD_INVILID, 0};
			header.flag = checkFlag;
			header.cmd = buffer->readInt16();
			header.datalen = buffer->readInt16();
			TLOG_DEBUG("header:" << header.flag << ", cmd:" << header.cmd << ", len:" << header.datalen);

			if(buffer->readableBytes() < header.datalen) {
				TLOG_DEBUG("Readable bytes : " << buffer->readableBytes() << " less than header's datalen: " << header.datalen);
				buffer->prependInt16(header.datalen);
				buffer->prependInt16(header.cmd);
				buffer->prependInt16(header.flag);
			} else {
				TLOG_DEBUG("Readable bytes : " << buffer->readableBytes());
				switch (header.cmd)
				{
				case CMD_JOINROOM:
					{
						if(buffer->readableBytes() < sizeof(uint32_t)) {
							TLOG_DEBUG("Invalid data len");
							buffer->retrieveAll();
						} else {
							auto roomId = buffer->readInt32();
							TLOG_DEBUG("Command join room: " << roomId);
							auto room = gController.getRoomById(roomId);
							auto ps = gController.getPlayerSessionById(tcpConnPtr->id());
							if(room != nullptr && ps != nullptr) {
								ps->joinRoom(room);
							}
						}
					}
					break;
				case CMD_EXITROOM:
					{
						auto ps = gController.getPlayerSessionById(tcpConnPtr->id());
						if(ps != nullptr) {
							ps->exitRoom();
						}
					}
					break;
				
				default:
					TLOG_DEBUG("Invalid Command!");
					tcpConnPtr->send("Invalid Command!");
					break;
				}
			}
		}
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
	server.setDisconnectionCallback(onDisconnection);
	server.setMessageCallback(onNewMessage);
	server.start();
	loop.loop();
	return 0;
}