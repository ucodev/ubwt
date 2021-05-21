#!/bin/sh

TEST_NAME="random"
TEST_VER="1"

if [ -e "/dev/urandom" ]; then
	TEST_OPTS="-b -C /dev/urandom -B 128000"
elif [ -e "/dev/random" ]; then
	TEST_OPTS="-b -C /dev/random -B 128000"
else
	echo "Unable to perform the test: ${TEST_NAME}"
	exit 1
fi

. ./libtest.in

