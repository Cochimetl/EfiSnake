#!/bin/sh

REPO_ROOT=$(git rev-parse --show-toplevel)

BUILD_DIR=${REPO_ROOT}/build

if [ ! -d $BUILD_DIR ]; then
	mkdir $BUILD_DIR
fi

cmake -B ${BUILD_DIR}/ -S ${REPO_ROOT}/

cmake --build ${BUILD_DIR}


