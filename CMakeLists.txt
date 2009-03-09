cmake_minimum_required (VERSION 2.6)

project (HOLDINGNUTS)

# switch features on/off
option (WITHOUT_CLIENT "do not build the client" Off)
option (WITHOUT_SERVER "do not build the server" Off)
option (WITHOUT_TEST "do not build the test-utils" Off)

# check for inner-source-build
STRING (COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" in_source)
IF (in_source)
	MESSAGE( "Inner-source-builds should not be used. For an outer-source-build create a separate build-directory and run cmake within it again.") 
ENDIF (in_source)

# default build-type is DEBUG
IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Build types are: None Debug Release RelWithDebInfo MinSizeRel" FORCE)
ENDIF (NOT CMAKE_BUILD_TYPE)

################################################################################

if (UNIX)
	set(PLATFORM_STR  "unix")
	add_definitions(-DPLATFORM_UNIX)
endif (UNIX)
if (WIN32)
	set(PLATFORM_STR  "win32")
	add_definitions(-DPLATFORM_WINDOWS)
endif (WIN32)
if (APPLE)
	set(PLATFORM_STR  "macosx")
	add_definitions(-DPLATFORM_MACOSX)
endif (APPLE)
IF (NOT PLATFORM_STR)
 set(PLATFORM_STR  "unix")
 MESSAGE("No target platform specified, using default (${PLATFORM_STR})")
 add_definitions(-DPLATFORM_UNIX)
ENDIF (NOT PLATFORM_STR)

#MESSAGE("Target platform is ${PLATFORM_STR}")

option (UPDATE_TRANSLATIONS "Update source translations")


# Doxygen documentation
if (BUILD_DOCUMENTATION)
	add_subdirectory (doxygen)
endif (BUILD_DOCUMENTATION)


################################################################################

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG=1)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

add_definitions(-Wall)  #-pedantic -fstack-protector -fstack-protector-all

################################################################################

add_subdirectory (src)
