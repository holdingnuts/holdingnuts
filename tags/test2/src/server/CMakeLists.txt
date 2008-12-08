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
		COMMAND i686-mingw32-windres
		-I${CMAKE_CURRENT_SOURCE_DIR}
		-o ${CMAKE_CURRENT_BINARY_DIR}/pserver_res.obj
		< ${CMAKE_CURRENT_SOURCE_DIR}/pserver.rc
	)
ENDIF (WIN32)

IF (WIN32)
	add_executable (pserver pserver.cpp pserver_res.obj game.cpp)
	target_link_libraries(pserver Poker Network System ws2_32)
ELSE (WIN32)
	add_executable (pserver pserver.cpp game.cpp)
	target_link_libraries(pserver Poker Network System)
ENDIF (WIN32)
