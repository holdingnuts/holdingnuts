include_directories (
	${HOLDINGNUTS_SOURCE_DIR}/src
	${HOLDINGNUTS_SOURCE_DIR}/src/system
)

if (ENABLE_SQLITE)
	include_directories(${SQLITE3_INCLUDE_DIR})
endif (ENABLE_SQLITE)

add_library(Network Network.c)
add_library(SysAccess SysAccess.c)
add_library(System Tokenizer.cpp ConfigParser.cpp Logger.c)

if (ENABLE_SQLITE)
	add_library(Database Database.cpp)
endif (ENABLE_SQLITE)
