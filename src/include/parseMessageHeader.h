#ifndef GSERVER_COMMON_PARSEMESSAGEHEADER_H
#define GSERVER_COMMON_PARSEMESSAGEHEADER_H

#include <assert.h>
#include <tinyserver/buffer.h>
#include "gserver.h"

namespace gserver
{
void readHeaderFromBuffer(tinyserver::Buffer *buffer, MessageHeader& header)
{
	assert(buffer != nullptr);
	assert(buffer->readableBytes() >= sizeof(MessageHeader));
	header.flag = buffer->readInt16();
	header.cmd = (Command)buffer->readInt16();
	header.reqid = buffer->readInt32();
	header.datalen = buffer->readInt16();
	header.rspcode = (RspCode)buffer->readInt16();
	header.clientversion = buffer->readInt32();
}

void writeHeaderToBuffer(tinyserver::Buffer *buffer, MessageHeader& header)
{
	assert(buffer != nullptr);
	buffer->prependInt32(header.clientversion);
	buffer->prependInt16((uint16_t)header.rspcode);
	buffer->prependInt16(header.datalen);
	buffer->prependInt32(header.reqid);
	buffer->prependInt16((uint16_t)header.cmd);
	buffer->prependInt16(header.flag);
}

bool parseMessageHeader(tinyserver::Buffer *buffer, MessageHeader& header, std::string *errmsg)
{
	if(buffer->readableBytes() < sizeof(MessageHeader)) {
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
				writeHeaderToBuffer(buffer, header);
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
}

#endif // GSERVER_COMMON_PARSEMESSAGEHEADER_H