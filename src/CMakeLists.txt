add_executable(gserver
	main.cpp
)
target_link_libraries(gserver
	tinyserver
	pthread
)