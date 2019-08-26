#ifndef GSERVER_COMMON_PARSEMESSAGEHEADER_H
#define GSERVER_COMMON_PARSEMESSAGEHEADER_H

#include "gserver.h"
#include <string>

namespace tinyserver
{
	class Buffer;
}

namespace gserver
{
void readHeaderFromBuffer(tinyserver::Buffer *buffer, MessageHeader& header);
void writeHeaderToBuffer(tinyserver::Buffer *buffer, MessageHeader& header);
bool parseMessageHeader(tinyserver::Buffer *buffer, MessageHeader& header, std::string *errmsg);
}

#endif // GSERVER_COMMON_PARSEMESSAGEHEADER_H