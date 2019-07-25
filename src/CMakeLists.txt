set(proto_SRCS
	proto/playerpb.pb.cc
	proto/roompb.pb.cc
)

add_executable(gserver
	main.cpp
	Controller.cpp
	GServer.cpp
	PlayerSession.cpp
	Room.cpp
	${proto_SRCS}
)
target_link_libraries(gserver
	tinyserver
	pthread
	protobuf
)