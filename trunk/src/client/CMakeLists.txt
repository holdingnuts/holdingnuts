include_directories (
	${POKER_SOURCE_DIR}/src
	${POKER_SOURCE_DIR}/src/network
	${POKER_SOURCE_DIR}/src/libpoker
	${POKER_SOURCE_DIR}/src/client
	
)
link_directories(${POKER_SOURCE_DIR}/src/network ${POKER_SOURCE_DIR}/src/libpoker)

IF (WIN32)
	# http://sources.redhat.com/binutils/docs-2.15/binutils/windres.html
	# http://www.resedit.net
	ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pclient_t_res.obj
		COMMAND i686-mingw32-windres
		-I${CMAKE_CURRENT_SOURCE_DIR}
		-o ${CMAKE_CURRENT_BINARY_DIR}/pclient_t_res.obj
		< ${CMAKE_CURRENT_SOURCE_DIR}/pclient_t.rc
	)
ENDIF (WIN32)

IF (WIN32)
	add_executable (pclient_t pclient_t.cpp pclient_t_res.obj)
	target_link_libraries(pclient_t Poker Network ws2_32)
ELSE (WIN32)
	add_executable (pclient_t pclient_t.cpp)
	target_link_libraries(pclient_t Poker Network)
ENDIF (WIN32)
