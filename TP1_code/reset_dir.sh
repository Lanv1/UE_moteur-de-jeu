#!/bin/bash
# USAGE : ./reset_dir
# rebuilds project with right directory

rm -rf build
mkdir build
cd build
cmake ..