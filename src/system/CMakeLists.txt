include_directories (${HOLDINGNUTS_SOURCE_DIR}/src ${HOLDINGNUTS_SOURCE_DIR}/src/system)

add_library(Network Network.c)
add_library(SysAccess SysAccess.c)
add_library(System Tokenizer.cpp ConfigParser.cpp Logger.c)
