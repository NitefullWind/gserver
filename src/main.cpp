#include <tinyserver/eventLoop.h>
#include <tinyserver/inetAddress.h>
#include <tinyserver/tcp/tcpServer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>
#include <sys/resource.h>
#define CORE_SIZE   1024 * 1024 * 500

#include "GServer.h"

using namespace tinyserver;
using namespace gserver;

bool parseRequestHeader(Buffer *buffer, RequestHeader &header)
{
	if(buffer->readableBytes() < sizeof(RequestHeader)) {
		TLOG_DEBUG("Readable bytes less than " << sizeof(RequestHeader));
	} else {
		uint16_t checkFlag = buffer->readInt16();
		if(checkFlag != RequestHeaderFlag) {
			TLOG_WARN("Invalid request header flag:" << checkFlag);
			buffer->retrieve(sizeof(RequestHeader));
		} else {
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
				return true;
			}
		}
	}
	return false;
}

int main(int argc, char **argv)
{
    struct rlimit rlmt;
    rlmt.rlim_cur = (rlim_t)CORE_SIZE;
    rlmt.rlim_max  = (rlim_t)CORE_SIZE;
	if (setrlimit(RLIMIT_CORE, &rlmt) == -1) {
		return -1; 
	}

	Logger::SetLevel(Logger::Debug);
	
	EventLoop loop;
	TcpServer server(&loop, InetAddress(8086));
	server.setIOThreadNum(2);

	gserver::GServer gserver;
	server.setConnectionCallback([&](const TcpConnectionPtr& connPtr) {
		gserver.onNewConnection(connPtr);
	});
	server.setDisconnectionCallback([&](const TcpConnectionPtr& connPtr) {
		gserver.onDisconnection(connPtr);
	});
	server.setMessageCallback([&](const TcpConnectionPtr& connPtr, Buffer *buffer) {
		RequestHeader header = {0, Command::INVILID, 0};
		if(parseRequestHeader(buffer, header)) {
			gserver.onNewMessage(std::move(header), connPtr, buffer->read(header.datalen));
		}
	});

	server.start();
	loop.loop();
	return 0;
}