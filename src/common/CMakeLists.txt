add_library(gserver_common
	parseMessageHeader.cpp
)

target_link_libraries(gserver_common
	tinyserver
)