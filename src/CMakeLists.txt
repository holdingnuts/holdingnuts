#determine platform we are building on/for
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

################################################################################

IF (WIN32)
IF (MINGW)
	#MESSAGE("using windres resource compiler")
	# http://sourceware.org/binutils/docs/binutils/windres.html
	FIND_PROGRAM(WINDRES NAMES windres i686-mingw32-windres)
ELSE (MINGW)
	MESSAGE("using rc resource compiler")
	# FIXME: find VC resource compiler
ENDIF (MINGW)
ENDIF (WIN32)

################################################################################

# gather information about svn-revision
IF (USE_SVNREV)
	FIND_PROGRAM(SVNVERSION NAMES svnversion)
	IF (SVNVERSION)
		EXEC_PROGRAM (${SVNVERSION} ARGS ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE REVISION)
		MESSAGE ("Using svn revision string: ${REVISION}")
		
		# DIRTY and dangerous workaround to determine if svnversion returned a revision;
		# svnversion always returns 0 as exit-code... so it's hard to find out
		STRING(LENGTH ${REVISION} revlen)
		IF (revlen LESS 6)    # localized string "exported"
			add_definitions(-DSVN_REV="${REVISION}")
		ENDIF (revlen LESS 6)
	ENDIF (SVNVERSION)
ENDIF (USE_SVNREV)


if (ENABLE_DEBUG OR CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG=1)
endif (ENABLE_DEBUG OR CMAKE_BUILD_TYPE STREQUAL "Debug")


# compiled in data-dir path
if (CMAKE_DATA_PATH)
	add_definitions(-DDATA_DIR=\"${CMAKE_DATA_PATH}/holdingnuts/data\")
	message ("Using data-dir ${CMAKE_DATA_PATH}/holdingnuts/data")
else (CMAKE_DATA_PATH)
	SET (CMAKE_DATA_PATH "${CMAKE_INSTALL_PREFIX}/share")
endif (CMAKE_DATA_PATH)


# additional definitions
add_definitions(-Wall)

# do not include RPATHs to our static libs in binaries
#set (CMAKE_BUILD_WITH_INSTALL_RPATH false)
set (CMAKE_SKIP_BUILD_RPATH true)


################################################################################

# libraries
add_subdirectory (libpoker)
add_subdirectory (system)

# the server
IF (ENABLE_SERVER)
	add_subdirectory (server)
ENDIF (ENABLE_SERVER)

# the client (depends on Qt4 and SDL)
IF (ENABLE_CLIENT)
	add_subdirectory (client)
ENDIF (ENABLE_CLIENT)

# test utils
IF (ENABLE_TEST)
	add_subdirectory (test)
ENDIF (ENABLE_TEST)
