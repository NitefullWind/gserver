#include <assert.h>
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

void readHeaderFromBuffer(Buffer *buffer, MessageHeader& header)
{
	assert(buffer != nullptr);
	assert(buffer->readableBytes() >= sizeof(MessageHeader));
	header.flag = buffer->readInt16();
	header.cmd = buffer->readInt16();
	header.reqid = buffer->readInt32();
	header.datalen = buffer->readInt16();
	header.rspcode = buffer->readInt16();
	header.clientversion = buffer->readInt32();
}

void writeHeaderToBuffer(Buffer *buffer, MessageHeader& header)
{
	assert(buffer != nullptr);
	buffer->prependInt32(header.clientversion);
	buffer->prependInt16(header.rspcode);
	buffer->prependInt16(header.datalen);
	buffer->prependInt32(header.reqid);
	buffer->prependInt16(header.cmd);
	buffer->prependInt16(header.flag);
}

bool parseMessageHeader(Buffer *buffer, MessageHeader& header)
{
	if(buffer->readableBytes() < sizeof(MessageHeader)) {
		TLOG_DEBUG("Readable bytes less than " << sizeof(MessageHeader));
	} else {
		readHeaderFromBuffer(buffer, header);
		if(header.flag != MessageHeaderFlag) {
			TLOG_WARN("Invalid request header flag:" << header.flag);
			buffer->retrieveAll();
		} else {
			TLOG_DEBUG("header:" << header.flag << ", cmd:" << header.cmd << ", len:" << header.datalen);

			if(buffer->readableBytes() < header.datalen) {
				writeHeaderToBuffer(buffer, header);
				TLOG_DEBUG("Readable bytes : " << buffer->readableBytes() << " less than header's datalen: " << header.datalen);
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
		MessageHeader header = {0, Command::INVILID, 0, 0, RspCode::SUCCESS, 0};
		Buffer rspBuffer;
		if(parseMessageHeader(buffer, header)) {
			gserver.processRequest(header, connPtr, buffer->read(header.datalen), &rspBuffer);
		}
		header.datalen = static_cast<uint16_t>(rspBuffer.readableBytes());
		writeHeaderToBuffer(&rspBuffer, header);
		connPtr->send(&rspBuffer);
	});

	server.start();
	loop.loop();
	return 0;
}