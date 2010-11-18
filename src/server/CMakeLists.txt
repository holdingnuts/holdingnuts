include_directories (
	${HOLDINGNUTS_SOURCE_DIR}/src
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
	${HOLDINGNUTS_SOURCE_DIR}/src/system
	${HOLDINGNUTS_SOURCE_DIR}/src/server
)

if (ENABLE_SQLITE)
	include_directories(${SQLITE3_INCLUDE_DIR})
endif (ENABLE_SQLITE)

link_directories(
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
	${HOLDINGNUTS_SOURCE_DIR}/src/system
)

IF (WIN32)
	IF (MINGW)
		LIST(APPEND aux_lib ws2_32)
		
		ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pserver_res.obj
			COMMAND ${WINDRES}
			-I${CMAKE_CURRENT_SOURCE_DIR}
			-o ${CMAKE_CURRENT_BINARY_DIR}/pserver_res.obj
			< ${CMAKE_CURRENT_SOURCE_DIR}/pserver.rc
		)
		LIST(APPEND aux_obj pserver_res.obj)
	ELSE (MINGW)
		LIST(APPEND aux_lib wsock32)
		
		# FIXME: use VC resource compiler rc.exe
	ENDIF (MINGW)
ENDIF (WIN32)

# FIXME: Which one is the correct define for Haiku?
IF (BEOS)
	LIST(APPEND aux_lib network)
ENDIF (BEOS)

if (ENABLE_SQLITE)
       LIST (APPEND aux_lib Database)
       LIST (APPEND aux_lib ${SQLITE3_LIBRARIES})
endif (ENABLE_SQLITE)


add_executable (holdingnuts-server
	pserver.cpp ${aux_obj}
	game.cpp GameController.cpp Table.cpp ranking.cpp
)

target_link_libraries(holdingnuts-server
	Poker Network SysAccess System
	${aux_lib}
)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/holdingnuts-server DESTINATION
	        ${CMAKE_INSTALL_PREFIX}/bin)
