include_directories (
	${HOLDINGNUTS_SOURCE_DIR}/src
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
	${HOLDINGNUTS_SOURCE_DIR}/src/system
	${HOLDINGNUTS_SOURCE_DIR}/src/server
)
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


add_executable (holdingnuts-server
	pserver.cpp ${aux_obj}
	game.cpp GameController.cpp Table.cpp
)

target_link_libraries(holdingnuts-server
	Poker Network SysAccess System
	${aux_lib}
)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/holdingnuts-server DESTINATION
	        ${CMAKE_INSTALL_PREFIX}/bin)
