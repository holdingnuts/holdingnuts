#!/bin/bash

# clean all temporary cmake-files when using inner-source build
# outer-source build is the preferred method:
#     $ mkdir build ; cd build ; cmake ../trunk

make clean   2>/dev/null
rm -r {.,src}/{,libpoker,server,client,test,system}/{Makefile,CMakeCache.txt,CMakeFiles,cmake_install.cmake}   2>/dev/null
