#!/bin/bash

echo "Setting up build directory"
BUILD_DIR="$(dirname "$0")/build/"
mkdir -p "$BUILD_DIR"

echo "Compiling with g++"
g++ ./src/unity.cpp \
    -std=c++20 \
    -O0 \
    -g \
    -Wall -Wpedantic \
    -o "$BUILD_DIR/main"

echo "Running"
$BUILD_DIR/main
