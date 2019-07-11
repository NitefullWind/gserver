#include "Room.h"

using namespace gserver;

Room::Room(const std::string& name) :
	_name(name)
{
}

Room::Room(std::string&& name) :
	_name(std::move(name))
{
}

Room::~Room()
{
}