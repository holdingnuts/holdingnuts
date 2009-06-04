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

add_executable (gc_test
	gc_test.cpp
	../server/GameController.cpp
	../server/Table.cpp
	TestCase.cpp
)
target_link_libraries(gc_test Poker System)

add_executable (test
	test.cpp
)
target_link_libraries(test Poker System)

add_executable (simulator simulator.cpp)
target_link_libraries(simulator Poker)

add_executable (systest system.cpp)
target_link_libraries(systest System SysAccess)

