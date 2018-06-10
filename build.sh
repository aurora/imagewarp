#!/bin/bash

if [[ "$1" = "" ]]; then
    echo "usage: build.s <cpp-file>"
    exit 1
fi

dir=$(dirname "$1")
base=$(basename "$1" ".cpp")

CPPFLAGS="-I/usr/local/include/opencv -I/usr/local/include/opencv2"
CXXFLAGS="--std=c++1z -stdlib=libc++"
LDFLAGS="-L/usr/local/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_calib3d"

g++ $CPPFLAGS $CXXFLAGS $LDFLAGS -o "$dir/$base" "$1"
