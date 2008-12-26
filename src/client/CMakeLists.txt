include_directories (
	${HOLDINGNUTS_SOURCE_DIR}/src
	${HOLDINGNUTS_SOURCE_DIR}/src/network
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
	${HOLDINGNUTS_SOURCE_DIR}/src/system
	${HOLDINGNUTS_SOURCE_DIR}/src/server
	${HOLDINGNUTS_SOURCE_DIR}/src/client
)
link_directories(
	${HOLDINGNUTS_SOURCE_DIR}/src/network
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
	${HOLDINGNUTS_SOURCE_DIR}/src/system
)

IF (WIN32)
	IF (MINGW)
		LIST(APPEND aux_lib ws2_32)
		
		ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pclient_t_res.obj
			COMMAND ${WINDRES}
			-I${CMAKE_CURRENT_SOURCE_DIR}
			-o ${CMAKE_CURRENT_BINARY_DIR}/pclient_t_res.obj
			< ${CMAKE_CURRENT_SOURCE_DIR}/pclient_t.rc
		)
		
		LIST(APPEND aux_obj pclient_t_res.obj)
	ELSE (MINGW)
		LIST(APPEND aux_lib wsock32)
		
		# FIXME: use VC resource compiler rc.exe
	ENDIF (MINGW)
ENDIF (WIN32)

add_executable (
	pclient_t
	pclient_t.cpp ${aux_obj}
)

target_link_libraries(
	pclient_t
	Poker Network System
	${aux_lib}
)
