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

add_subdirectory (libpoker)
add_subdirectory (server)
add_subdirectory (client)
add_subdirectory (test)
add_subdirectory (system)

