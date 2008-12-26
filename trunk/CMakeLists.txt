cmake_minimum_required (VERSION 2.6)

set(DEBUG     1)

################################################################################

if (UNIX)
	set(PLATFORM_STR  "unix")
	add_definitions(-DPLATFORM_UNIX)
endif (UNIX)
if (WIN32)
	set(PLATFORM_STR  "win32")
	add_definitions(-DPLATFORM_WINDOWS)
endif (WIN32)
if (MACOSX)
	set(PLATFORM_STR  "macosx")
	add_definitions(-DPLATFORM_MACOSX)
endif (MACOSX)
IF (NOT PLATFORM_STR)
 set(PLATFORM_STR  "unix")
 MESSAGE("No target platform specified, using default (${PLATFORM_STR})")
 add_definitions(-DPLATFORM_UNIX)
ENDIF (NOT PLATFORM_STR)

MESSAGE("Target platform is ${PLATFORM_STR}")

################################################################################

if (DEBUG)
	add_definitions(-g -O0 -DDEBUG=${DEBUG})
else (DEBUG)
	add_definitions(-O2 -fstack-protector -fstack-protector-all)
endif (DEBUG)

add_definitions(-Wall)  #-pedantic #-DPLATFORM_STR=${PLATFORM_STR}

################################################################################

project (HOLDINGNUTS)

add_subdirectory (src)


