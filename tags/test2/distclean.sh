#!/bin/bash

# clean all temporary cmake-files

make clean   2>/dev/null
rm -r {.,src}/{,network,libpoker,server,client,test,system}/{Makefile,CMakeCache.txt,CMakeFiles,cmake_install.cmake}   2>/dev/null
