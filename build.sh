#!/bin/sh

set -x

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-./build}
BUILD_TYPE=${BUILD_TYPE:-debug}

mkdir -p $BUILD_DIR/$BUILD_TYPE \
	&& cd $BUILD_DIR/$BUILD_TYPE \
	&& cmake \
		-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
		$SOURCE_DIR \
	&& make $*	
