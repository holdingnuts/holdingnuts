cmake_minimum_required (VERSION 2.6)

project (HOLDINGNUTS)

# switch features on/off
option (ENABLE_CLIENT	"Configure for client"		On)
option (ENABLE_AUDIO	"Configure client with audio"	On)
option (ENABLE_SERVER	"Configure for server"		On)
option (ENABLE_TEST	"Configure for test-utils"	Off)
option (ENABLE_DEBUG	"Configure for debug-build"	Off)

option (USE_SVNREV "Include the svn-revision in build" Off)
option (UPDATE_TRANSLATIONS "Update source translations" Off)
#OPTION(BUILD_DOCUMENTATION "Build the documentation (Doxygen)." OFF)   # this one is provided by CMake

################################################################################

# check for inner-source-build
STRING (COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" in_source)
IF (in_source)
	MESSAGE( "Inner-source-builds should not be used. For an outer-source-build create a separate build-directory and run cmake within it again.") 
ENDIF (in_source)

################################################################################

# Doxygen documentation
if (BUILD_DOCUMENTATION)
	add_subdirectory (doxygen)
endif (BUILD_DOCUMENTATION)

# the sources
add_subdirectory (src)
