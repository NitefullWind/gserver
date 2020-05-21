#include "parseMessageHeader.h"

#include <assert.h>
#include <tinyserver/buffer.h>
#include <tinyserver/logger.h>
#include <tinyserver/tcp/tcpConnection.h>

#include "../auth/PlayerSession.h"

namespace gserver
{
void readHeaderFromBuffer(tinyserver::Buffer *buffer, MessageHeader& header)
{
	assert(buffer != nullptr);
	assert(buffer->readableBytes() >= MessageHeaderLength);
	header.flag = buffer->readUInt16();
	header.cmd = (Command)buffer->readUInt16();
	header.reqid = buffer->readUInt32();
	header.datalen = buffer->readUInt32();
	header.rspcode = (RspCode)buffer->readUInt16();
	header.clientversion = buffer->readUInt32();
}

void writeHeaderToBuffer(tinyserver::Buffer *buffer, const MessageHeader& header)
{
	assert(buffer != nullptr);
	buffer->prependUInt32(header.clientversion);
	buffer->prependUInt16((uint16_t)header.rspcode);
	buffer->prependUInt32(header.datalen);
	buffer->prependUInt32(header.reqid);
	buffer->prependUInt16((uint16_t)header.cmd);
	buffer->prependUInt16(header.flag);
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

void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, const MessageHeader& header, tinyserver::Buffer *buffer)
{
	assert(tcpConnPtr != nullptr);
	writeHeaderToBuffer(buffer, header);
	tcpConnPtr->send(buffer);
}

void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, Command cmd, uint32_t reqid, tinyserver::Buffer *buffer)
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


bool sendMessageToPSPtr(const std::shared_ptr<gserver::PlayerSession>& playerPtr,
						Command cmd, uint32_t reqid, const std::string& data, std::string *const errmsg)
{
	auto tcpPtr = playerPtr->tcpConnectionWeakPtr().lock();
	if (!tcpPtr) {
		if (errmsg) {
			*errmsg = "TCP链接不存在";
		}
		return false;
	} else {
		sendMessageToConnection(tcpPtr, cmd, reqid, data);
		return true;
	}
	return false;
}

bool sendMessageToPSWPtr(const std::weak_ptr<gserver::PlayerSession>& playerWPtr,
						Command cmd, uint32_t reqid, const std::string& data, std::string *const errmsg)
{
	auto psPtr = playerWPtr.lock();
	if (!psPtr) {
		if (errmsg) {
			*errmsg = "用户指针不存在";
		}
	} else {
		auto tcpPtr = psPtr->tcpConnectionWeakPtr().lock();
		if (!tcpPtr) {
			if (errmsg) {
				*errmsg = "用户TCP链接不存在";
			}
			return false;
		} else {
			sendMessageToConnection(tcpPtr, cmd, reqid, data);
			return true;
		}
	}
	return false;
}

}