# Cross compile for windows
#
# configure with "cmake -DCMAKE_TOOLCHAIN_FILE=$(pwd)/Toolchain_Windows.cmake ."

SET(CMAKE_SYSTEM_NAME Windows)
#SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_C_COMPILER   i686-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-mingw32-g++)
#SET(CMAKE_AR i686-mingw32-ar)   # huh, bug in link.txt when setting this one
SET(CMAKE_RANLIB i686-mingw32-ranlib)
