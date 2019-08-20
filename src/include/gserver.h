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
	uint16_t datalen;
	RspCode rspcode;
	uint32_t clientversion;
} MessageHeader;

#define MessageHeaderFlag 0x4753

}

#endif // GSERVER_COMMON_GSERVER_H