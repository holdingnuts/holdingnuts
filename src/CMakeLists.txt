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


IF (NOT WITHOUT_SERVER)
	add_subdirectory (server)
ENDIF (NOT WITHOUT_SERVER)

IF (NOT WITHOUT_CLIENT)
	add_subdirectory (client)
ENDIF (NOT WITHOUT_CLIENT)

IF (NOT WITHOUT_TEST)
	add_subdirectory (test)
ENDIF (NOT WITHOUT_TEST)
