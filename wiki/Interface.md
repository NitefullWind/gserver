[TOC]

# 协议
自定义消息头 + google-protobuf消息内容

也就是客户端与服务端的数据包内容，都必须带有一个消息头，再带有protobuf的消息内容

## 自定义消息头

消息头固定为18字节(72位)长度，按顺序定义如下：
- 16位校验标志
- 16位命令号
- 32位请求序列化
- 32位protobuf数据长度
- 16位响应代码
- 32位客户端版本号

C++中定义示例：
```cpp
typedef struct
{
	uint16_t flag;
	Command cmd;
	uint32_t reqid;
	uint32_t datalen;
	RspCode rspcode;
	uint32_t clientversion;
} MessageHeader;
```

## protobuf消息
使用proto3定义的消息体，客户端需要使用相同的proto文件编译为自己的语言并使用

目前定义了如下几个消息体
- [playerpb.proto](../src/proto/playerpb.proto) 玩家信息
- [roompb.proto](../src/proto/roompb.proto) 房间信息，描述游戏房间或聊天房间
- [chatmsgpb.proto](../src/proto/chatmsgpb.proto) 聊天房间的聊天信息

## 命令
支持的命令：
```cpp
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
```
注意：服务端和客户端共同使用这些命令，同一个命令既可能是请求命令，也可能是响应命令，跟具体命令的定义相关。比如对服务端来说，收到`ROOMINFO(105)`命令，代表着客户端想要查询房间信息，然后服务端会向该客户端返回一个`ROOMINFO(105)`命令，并带有房间信息的消息体，客户端收到该消息，意味着查询到了具体的房间信息。

### 发送命令
|  命令 | 功能 | 必要入参 | 服务端行为 |
| -------- | ------ | -------- | ---------- |
| LOGIN | 登录 | 用户名、密码 | 用户登录，返回当前房间列表 |
| LOGOUT | 登出 | 房间id | 用户登出 |
| CREATEROOM | 创建房间 | 房间名、描述、密码等 | 1.对游戏服务器：创建用户的游戏房间，创建对应的聊天房间，返回游戏房间信息。2.对聊天服务器：创建用户的聊天房间，返回聊天房间信息 |
| UPDATEROOM | 更新房间 | 房间名、描述、密码等 | 更新指定的房间 |
| JOINROOM | 加入房间 | 房间id | 将用户加入指定房间 |
| EXITROOM | 退出房间 | 房间id | 退出指定房间 |
| ROOMLIST | 查询房间列表 | 无 | 返回房间列表 |
| ROOMINFO | 查询房间信息 | 房间id | 返回指定的房间信息 |
| SENDMSG | 发送消息 | 接收者、消息内容等 | 将聊天内容发送给指定的接收者 |

### 收的命令
|  命令 | 功能 |
| -------- | ------ |
| LOGIN | 登录请求应答 |
| LOGOUT | 登出请求应答 |
| CREATEROOM | 创建房间请求应答 |
| UPDATEROOM | 更新房间请求应答，或房间信息变更通知，比如游戏房间的聊天房间被创建好 |
| JOINROOM | 加入房间请求应答 |
| EXITROOM | 退出房间请求应答 |
| ROOMLIST | 查询房间列表请求应答 |
| ROOMINFO | 查询房间信息请求应答 |
| SENDMSG | 发送消息请求应答 |
| RECVMSG | 收到消息 |

