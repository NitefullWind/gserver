#ifndef GSERVER_COMMON_GSERVER_H
#define GSERVER_COMMON_GSERVER_H

#include <stdint.h>

namespace gserver
{
enum class Command : uint16_t
{
	INVILID = 0,
	LOGIN = 1,
	LOGOUT = 2,
	CREATEROOM = 100,
	UPDATEROOM = 101,
	JOINROOM = 102,
	EXITROOM = 103,
	SENDMSG = 200,
	RECVMSG = 201,
};

enum class RspCode : uint16_t
{
	ERROR = 0,
	SUCCESS = 1,
};

typedef struct
{
	uint16_t flag;
	Command cmd;
	uint32_t reqid;
	uint32_t datalen;
	RspCode rspcode;
	uint32_t clientversion;
} MessageHeader;
// MessageHeader数据长度，因内存对其，可能不等于sizeof(MessageHeader)
#define MessageHeaderLength 18

#define MessageHeaderFlag 0x4753
#define MessageHeaderVersion 1

}

#endif // GSERVER_COMMON_GSERVER_H