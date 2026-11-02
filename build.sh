#!/bin/sh
# Linux/macOS build (used for testing): ./build.sh && ./studygroup_server
cd "$(dirname "$0")" && g++ -std=c++17 -O2 -Wall backend/*.cpp -o studygroup_server && echo built
