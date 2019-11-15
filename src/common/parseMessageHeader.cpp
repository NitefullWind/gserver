#include "parseMessageHeader.h"

#include <assert.h>
#include <tinyserver/buffer.h>
#include <tinyserver/logger.h>
#include <tinyserver/tcp/tcpConnection.h>

namespace gserver
{
void readHeaderFromBuffer(tinyserver::Buffer *buffer, MessageHeader& header)
{
	assert(buffer != nullptr);
	assert(buffer->readableBytes() >= MessageHeaderLength);
	header.flag = buffer->readInt16();
	header.cmd = (Command)buffer->readInt16();
	header.reqid = buffer->readInt32();
	header.datalen = buffer->readInt32();
	header.rspcode = (RspCode)buffer->readInt16();
	header.clientversion = buffer->readInt32();
}

void writeHeaderToBuffer(tinyserver::Buffer *buffer, const MessageHeader& header)
{
	assert(buffer != nullptr);
	buffer->prependInt32(header.clientversion);
	buffer->prependInt16((uint16_t)header.rspcode);
	buffer->prependInt32(header.datalen);
	buffer->prependInt32(header.reqid);
	buffer->prependInt16((uint16_t)header.cmd);
	buffer->prependInt16(header.flag);
}

bool parseMessageHeader(tinyserver::Buffer *buffer, MessageHeader& header, std::string *errmsg)
{
	if(buffer->readableBytes() < MessageHeaderLength) {
		if(errmsg) {
			*errmsg = "Too few readable bytes";
		}
	} else {
		readHeaderFromBuffer(buffer, header);
		if(header.flag != MessageHeaderFlag) {
			buffer->retrieveAll();
			if(errmsg) {
				*errmsg = "Invalid request header flag: " + std::to_string(header.flag);
			}
		} else {
			if(buffer->readableBytes() < header.datalen) {
				gserver::writeHeaderToBuffer(buffer, header);
				if(errmsg) {
					*errmsg = "Readable bytes: " + std::to_string(buffer->readableBytes()) + " less than header's datalen: " + std::to_string(header.datalen);
				}
			} else {
				return true;
			}
		}
	}
	return false;
}

void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, const MessageHeader& header, const tinyserver::Buffer *buffer)
{
	assert(tcpConnPtr != nullptr);
	tinyserver::Buffer sendBuf(*buffer);
	writeHeaderToBuffer(&sendBuf, header);
	tcpConnPtr->send(&sendBuf);
}

void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, Command cmd, uint32_t reqid, const tinyserver::Buffer *buffer)
{
	MessageHeader header = {MessageHeaderFlag, cmd, reqid, static_cast<uint32_t>(buffer->readableBytes()), RspCode::SUCCESS, MessageHeaderVersion};
	return sendMessageToConnection(tcpConnPtr, header, buffer);
}

void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, const MessageHeader& header, const std::string& data)
{
	tinyserver::Buffer sendBuf;
	sendBuf.append(data);
	writeHeaderToBuffer(&sendBuf, header);
	TLOG_DEBUG("Send message to connection: " << tcpConnPtr->id() << ", cmd: " << (int)header.cmd << ", data size: " << sendBuf.readableBytes());
	tcpConnPtr->send(&sendBuf);
}

void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, Command cmd, uint32_t reqid, const std::string& data)
{
	MessageHeader header = {MessageHeaderFlag, cmd, reqid, static_cast<uint32_t>(data.length()), RspCode::SUCCESS, MessageHeaderVersion};
	return sendMessageToConnection(tcpConnPtr, header, data);
}

}