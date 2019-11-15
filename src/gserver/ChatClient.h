#ifndef GSERVER_CHATCLIENT_H
#define GSERVER_CHATCLIENT_H

#include "common/gserver.h"

#include <tinyserver/eventLoop.h>
#include <tinyserver/tcp/TcpClient.h>

/**
 * @brief 聊天服务器的客户端
 */
namespace gserver
{
    class RoomPB;
    class UserManager;

    class ChatClient
    {
    public:
        ChatClient(tinyserver::EventLoop *loop, UserManager *userMgr);
        ~ChatClient();

        /**
         * @brief 启动客户端
         */
        void start();

        /**
         * @brief 创建聊天房间
         * 
         * @param roompb 错误信息
         * @param errmsg 错误信息
         * @return true 创建成功
         * @return false 创建失败
         */
        bool createRoom(RoomPB *roompb, std::string *errmsg = nullptr);
    private:
        tinyserver::TcpClient _tcpClient;
        bool _started;
        bool _isLogin;
		UserManager *_userMgr;

        /**
         * @brief 处理聊天服务器的响应数据
         * 
         * @param header 消息头
         * @param tcpConnPtr TCP连接
         * @param reqMsg 响应消息
         */
        void processResponse(MessageHeader& header, const tinyserver::TcpConnectionPtr& tcpConnPtr, const std::string& rspMsg);
    };
}

#endif // GSERVER_CHATCLIENT_H