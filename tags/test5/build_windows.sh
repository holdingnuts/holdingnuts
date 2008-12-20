#!/bin/bash

cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/Toolchain_Windows.cmake -DWIN32:BOOL=On .
make #VERBOSE=1

