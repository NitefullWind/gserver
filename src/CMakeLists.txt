set(proto_SRCS
	${PROJECT_SOURCE_DIR}/src/proto/message.pb.cc
	${PROJECT_SOURCE_DIR}/src/proto/playerpb.pb.cc
	${PROJECT_SOURCE_DIR}/src/proto/roompb.pb.cc
)

add_subdirectory(gserver)