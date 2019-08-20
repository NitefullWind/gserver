add_library(gserver_auth
	UserManager.cpp
	PlayerSession.cpp
	Room.cpp
	${proto_SRCS}
)

target_link_libraries(gserver_auth
	tinyserver
)