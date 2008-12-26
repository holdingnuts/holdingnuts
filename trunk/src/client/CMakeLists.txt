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
	# http://sources.redhat.com/binutils/docs-2.15/binutils/windres.html
	# http://www.resedit.net
	ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pclient_t_res.obj
		COMMAND ${WINDRES}
		-I${CMAKE_CURRENT_SOURCE_DIR}
		-o ${CMAKE_CURRENT_BINARY_DIR}/pclient_t_res.obj
		< ${CMAKE_CURRENT_SOURCE_DIR}/pclient_t.rc
	)
	
	LIST(APPEND aux_obj pclient_t_res.obj)
	LIST(APPEND aux_lib ws2_32)
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
