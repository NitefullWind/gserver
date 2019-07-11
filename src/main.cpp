#include <tinyserver/eventLoop.h>
#include <tinyserver/inetAddress.h>
#include <tinyserver/tcp/tcpServer.h>
#include <tinyserver/tcp/tcpConnection.h>
#include <tinyserver/logger.h>

using namespace tinyserver;

void onNewMessage(const TcpConnectionPtr &tcpConnPtr, Buffer *buffer)
{
	TLOG_DEBUG("new message from " << tcpConnPtr->peerAddress().toHostPort());
	tcpConnPtr->send(buffer);
}

int main(int argc, char **argv)
{
	Logger::SetLevel(Logger::Debug);
	EventLoop loop;
	TcpServer server(&loop, InetAddress(8086));
	server.setIOThreadNum(2);
	server.setMessageCallback(onNewMessage);
	server.start();
	loop.loop();
	return 0;
}