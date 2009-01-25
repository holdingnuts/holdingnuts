include_directories (
	${HOLDINGNUTS_SOURCE_DIR}/src
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
	${HOLDINGNUTS_SOURCE_DIR}/src/system
	${HOLDINGNUTS_SOURCE_DIR}/src/server
	${HOLDINGNUTS_SOURCE_DIR}/src/test
	
)
link_directories(
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
	${HOLDINGNUTS_SOURCE_DIR}/src/system
)

add_executable (test
	test.cpp
	../server/GameController.cpp
	../server/Table.cpp
)
target_link_libraries(test Poker)

add_executable (simulator simulator.cpp)
target_link_libraries(simulator Poker)

add_executable (systest system.cpp)
target_link_libraries(systest System SysAccess)


IF (WIN32)
	IF (MINGW)
		LIST(APPEND aux_lib ws2_32)
	ELSE (MINGW)
		LIST(APPEND aux_lib wsock32)
	ENDIF (MINGW)
ENDIF (WIN32)

add_executable ( pclient_t pclient_t.cpp )

target_link_libraries(
	pclient_t
	Poker Network System
	${aux_lib}
)
