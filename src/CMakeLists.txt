set(proto_SRCS
	${PROJECT_SOURCE_DIR}/src/proto/chatmsgpb.pb.cc
	${PROJECT_SOURCE_DIR}/src/proto/playerpb.pb.cc
	${PROJECT_SOURCE_DIR}/src/proto/roompb.pb.cc
)

include_directories(${PROJECT_SOURCE_DIR}/src/common)

add_subdirectory(auth)
add_subdirectory(common)
add_subdirectory(gserver)
add_subdirectory(ChatServer)