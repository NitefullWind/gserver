#include "PlayerSession.h"

#include <tinyserver/tcp/tcpConnection.h>

using namespace gserver;

PlayerSession::PlayerSession(std::shared_ptr<tinyserver::TcpConnection> connection) :
	_tcpConnection(connection)
{
}

PlayerSession::~PlayerSession()
{
}