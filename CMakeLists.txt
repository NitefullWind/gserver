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
	#-rdynamic
	-D_GLIBCXX_USE_CXX11_ABI=0
	)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	list(APPEND CXX_FLAGS "-Wno-null-dereference")
	list(APPEND CXX_FLAGS "-Wno-sign-conversion")
	list(APPEND CXX_FLAGS "-Wno-unused-local-typedef")
	list(APPEND CXX_FLAGS "-Wthread-safety")
	list(REMOVE_ITEM CXX_FLAGS "-rdynamic")
endif()
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/lib)

find_package(Protobuf)
if(PROTOBUF_FOUND)
	message(STATUS "found protobuf")
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/src)

link_directories(${PROJECT_SOURCE_DIR}/lib)
link_directories(/usr/local/lib)

add_subdirectory(src)
