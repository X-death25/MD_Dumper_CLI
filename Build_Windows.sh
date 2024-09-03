#!/bin/bash
# sudo apt-get install mingw-w64 g++-mingw-w64

cmake -Wno-dev -DINCLUDE_EXTRA_CMAKELISTS=ON -DCMAKE_TOOLCHAIN_FILE=../Toolchain-Ubuntu-mingw64.cmake .
make
mkdir Windows_Build
cp -dR LICENSE ./Windows_Build/
cp *.csv ./Windows_Build/
cp MD_Dumper.exe ./Windows_Build/
cp -dR images ./Windows_Build/
cp -dR MD_Dumper_GUI.bat ./Windows_Build/
cp ./windows_deps/files/bin/*.dll ./Windows_Build
rm -rf CMakeFiles
rm CMakeCache.txt
rm cmake_install.cmake
rm Makefile

