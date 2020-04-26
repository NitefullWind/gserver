#ifndef GSERVER_COMMON_PARSEMESSAGEHEADER_H
#define GSERVER_COMMON_PARSEMESSAGEHEADER_H

#include "gserver.h"
#include <memory>
#include <string>

namespace tinyserver
{
	class Buffer;
	class TcpConnection;
}

namespace gserver
{
void readHeaderFromBuffer(tinyserver::Buffer *buffer, MessageHeader& header);
void writeHeaderToBuffer(tinyserver::Buffer *buffer, const MessageHeader& header);
bool parseMessageHeader(tinyserver::Buffer *buffer, MessageHeader& header, std::string *errmsg);

void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, const MessageHeader& header, tinyserver::Buffer *buffer);
void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, Command cmd, uint32_t reqid, tinyserver::Buffer *buffer);
void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, const MessageHeader& header, const std::string& data);
void sendMessageToConnection(const std::shared_ptr<tinyserver::TcpConnection>& tcpConnPtr, Command cmd, uint32_t reqid, const std::string& data);
}

#endif // GSERVER_COMMON_PARSEMESSAGEHEADER_H