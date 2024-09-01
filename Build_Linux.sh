#!/bin/bash

cmake -Wno-dev ./
make
mkdir Linux_Build
cp *.csv ./Linux_Build/
cp MD_Dumper ./Linux_Build/
cp -dR images ./Linux_Build/
rm -rf CMakeFiles
rm CMakeCache.txt
rm cmake_install.cmake
rm Makefile
