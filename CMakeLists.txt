cmake_minimum_required(VERSION 2.6)

project(gserver CXX)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "debug")
endif()

set(CXX_FLAGS
	-g
	-Wall
	-Wextra
	-Werror
	#-Wconversion
	-Wno-unused-parameter
	-Wpointer-arith
	-march=native
	-std=c++11
	-rdynamic
	)
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/lib)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

link_directories(${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(src)
