#ifndef GSERVER_COMMON_GSERVER_H
#define GSERVER_COMMON_GSERVER_H

#include <stdint.h>

namespace gserver
{
enum class Command : uint16_t
{
	// 基础命令
	INVILID = 0,			// 非法命令
	LOGIN = 1,				// 登录
	LOGOUT = 2,				// 登出
	// 房间相关命令
	CREATEROOM = 100,		// 创建房间
	UPDATEROOM = 101,		// 更新房间
	JOINROOM = 102,			// 加入房间
	EXITROOM = 103,			// 退出房间
	ROOMLIST = 104,			// 房间列表
	ROOMINFO = 105,			// 房间信息
	// 聊天相关命令
	SENDMSG = 200,			// 发送消息
	RECVMSG = 201,			// 接收消息
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