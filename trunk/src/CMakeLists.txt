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
add_subdirectory (system)


IF (ENABLE_SERVER)
	add_subdirectory (server)
ENDIF (ENABLE_SERVER)

IF (ENABLE_CLIENT)
	add_subdirectory (client)
ENDIF (ENABLE_CLIENT)

IF (ENABLE_TEST)
	add_subdirectory (test)
ENDIF (ENABLE_TEST)
