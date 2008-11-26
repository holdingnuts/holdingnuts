include_directories (
	${POKER_SOURCE_DIR}/src
	${POKER_SOURCE_DIR}/src/network
	${POKER_SOURCE_DIR}/src/libpoker
	${POKER_SOURCE_DIR}/src/test
	
)
link_directories(${POKER_SOURCE_DIR}/src/network ${POKER_SOURCE_DIR}/src/libpoker)

add_executable (test test.cpp)
target_link_libraries(test Poker)

add_executable (simulator simulator.cpp)
target_link_libraries(simulator Poker)
