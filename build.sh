#!/bin/bash/

#build lib
cd compressionLib
echo "Compiling shared library..."
g++ -c -Wall -Werror -fpic compression.cpp -o compression.o
g++ -shared -o libcompression.so compression.o
sudo cp libcompression.so /usr/lib
cd ..

build_dir="build"
if [ -d "$build_dir" ]; then
    echo "Cleaning build directory..."
    rm -rf "$build_dir"/*
else
    echo "Creating build directory..."
    mkdir "$build_dir"
fi
cd "$build_dir"
echo "Creating make file..."
qmake ..
echo "Compiling..."
make
echo "================================================================================="
./ImageCompressionApp
cd ..
