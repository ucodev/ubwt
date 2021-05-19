#!/bin/sh

TEST_NAME="multithreaded random"
TEST_VER="1"
TEST_OPTS="-W 4 -b -C /dev/random -B 128000"

. ./libtest.in

