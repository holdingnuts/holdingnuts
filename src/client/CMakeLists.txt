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
		
		#ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pclient_res.obj
		#	COMMAND ${WINDRES}
		#	-I${CMAKE_CURRENT_SOURCE_DIR}
		#	-o ${CMAKE_CURRENT_BINARY_DIR}/pclient_res.obj
		#	< ${CMAKE_CURRENT_SOURCE_DIR}/pclient.rc
		#)
		
		LIST(APPEND aux_obj pclient_t_res.obj)
	ELSE (MINGW)
		LIST(APPEND aux_lib wsock32)
		
		# FIXME: use VC resource compiler rc.exe
	ENDIF (MINGW)
ENDIF (WIN32)


# CMAKE_LIBRARY_PATH=c:/mingw/lib
# CMAKE_INCLUDE_PATH=c:/mingw/include/SDL

##Find_Package ( SDL REQUIRED )
##Find_Package ( SDL_image REQUIRED )
Find_Package ( Qt4 REQUIRED )

# http://www.qtforum.org/article/26305/problem-running-configure-to-install-qt.html

INCLUDE(${QT_USE_FILE})

include_directories (
##	${SDL_INCLUDE_DIR}
	#${QT_INCLUDES}    # done by QT_USE_FILE
)

QT4_WRAP_CPP(sources_MOC pclient.hpp WMain.hpp WTable.hpp)

add_executable (
	pclient
	WIN32    # when building for windows: do not show the console window
	pclient.cpp WMain.cpp WTable.cpp ${sources_MOC}
)

target_link_libraries (
   pclient
   Poker Network System
   ${QT_LIBRARIES}
##   ${SDL_LIBRARY}
##   ${SDLIMAGE_LIBRARY}
##   SDLmain
)
