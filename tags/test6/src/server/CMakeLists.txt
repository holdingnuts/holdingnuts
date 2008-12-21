include_directories (
	${POKER_SOURCE_DIR}/src
	${POKER_SOURCE_DIR}/src/network
	${POKER_SOURCE_DIR}/src/libpoker
	${POKER_SOURCE_DIR}/src/system
	${POKER_SOURCE_DIR}/src/server
)
link_directories(
	${POKER_SOURCE_DIR}/src/network
	${POKER_SOURCE_DIR}/src/libpoker
	${POKER_SOURCE_DIR}/src/system
)

IF (WIN32)
	# http://sources.redhat.com/binutils/docs-2.15/binutils/windres.html
	# http://www.resedit.net
	ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pserver_res.obj
		COMMAND ${WINDRES}
		-I${CMAKE_CURRENT_SOURCE_DIR}
		-o ${CMAKE_CURRENT_BINARY_DIR}/pserver_res.obj
		< ${CMAKE_CURRENT_SOURCE_DIR}/pserver.rc
	)
	
	LIST(APPEND aux_obj pserver_res.obj)
	LIST(APPEND aux_lib ws2_32)
ENDIF (WIN32)


add_executable (pserver
	pserver.cpp ${aux_obj}
	game.cpp GameController.cpp Table.cpp
)

target_link_libraries(pserver
	Poker Network System
	${aux_lib}
)
