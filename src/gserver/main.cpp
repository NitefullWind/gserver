#include <assert.h>
#include <tinyserver/eventLoop.h>
#include <tinyserver/inetAddress.h>
#include <tinyserver/tcp/tcpServer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>
#include <sys/resource.h>

#include "GServer.h"
#include "parseMessageHeader.h"

using namespace tinyserver;
using namespace gserver;

#define CORE_SIZE   1024 * 1024 * 500

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
		MessageHeader header = {0, Command::INVILID, 0, 0, RspCode::SUCCESS, 0};
		Buffer rspBuffer;
		rspBuffer.setPrependSize(MessageHeaderLength);
		std::string errmsg;
		if(parseMessageHeader(buffer, header, &errmsg)) {
			gserver.processRequest(header, connPtr, buffer->read(header.datalen), &rspBuffer);
		} else {
			TLOG_INFO(errmsg);
		}
		header.datalen = static_cast<uint32_t>(rspBuffer.readableBytes());
		writeHeaderToBuffer(&rspBuffer, header);
		connPtr->send(&rspBuffer);
	});

	server.start();
	loop.loop();
	return 0;
}