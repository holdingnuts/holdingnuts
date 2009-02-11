include_directories (
	${HOLDINGNUTS_SOURCE_DIR}/src
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
	${HOLDINGNUTS_SOURCE_DIR}/src/system
	${HOLDINGNUTS_SOURCE_DIR}/src/server
	${HOLDINGNUTS_SOURCE_DIR}/src/client
)
link_directories(
	${HOLDINGNUTS_SOURCE_DIR}/src/libpoker
	${HOLDINGNUTS_SOURCE_DIR}/src/system
)

IF (WIN32)
	IF (MINGW)
		LIST(APPEND aux_lib ws2_32)
		
		ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pclient_res.obj
			COMMAND ${WINDRES}
			-I${CMAKE_CURRENT_SOURCE_DIR}
			-o ${CMAKE_CURRENT_BINARY_DIR}/pclient_res.obj
			< ${CMAKE_CURRENT_SOURCE_DIR}/pclient.rc
		)
		
		LIST(APPEND aux_obj pclient_res.obj)
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

SET(QtApp_RCCS pclient.qrc)

QT4_ADD_RESOURCES(QtApp_RCC_SRCS ${QtApp_RCCS})

QT4_WRAP_CPP(sources_MOC pclient.hpp WMain.hpp WTable.hpp ChatBox.hpp DealerButton.hpp Seat.hpp EditableSlider.hpp TimeOut.hpp)

add_executable (
	pclient
	WIN32    # when building for windows: do not show the console window
	pclient.cpp ${aux_obj} WMain.cpp WTable.cpp ChatBox.cpp DealerButton.cpp Seat.cpp SeatImages.cpp EditableSlider.cpp TimeOut.cpp ${sources_MOC} ${QtApp_RCC_SRCS}
)

target_link_libraries (
   pclient
   Poker Network SysAccess System
   ${QT_LIBRARIES}
   ${aux_lib}
##   ${SDL_LIBRARY}
##   ${SDLIMAGE_LIBRARY}
##   SDLmain
)

############################################################

#http://www.cmake.org/Wiki/CMake:How_To_Build_Qt4_Software
#http://doc.trolltech.com/4.3/i18n.html

file (GLOB TRANSLATIONS_FILES translations/*.ts)

file (GLOB SRC_FILES *.cpp *.hpp)

set (FILES_TO_TRANSLATE ${SRC_FILES})

if (UPDATE_TRANSLATIONS)
  qt4_create_translation(QM_FILES ${FILES_TO_TRANSLATE} ${TRANSLATIONS_FILES})
else (UPDATE_TRANSLATIONS)
  qt4_add_translation(QM_FILES ${TRANSLATIONS_FILES})
endif (UPDATE_TRANSLATIONS) 

add_custom_target (translations DEPENDS ${QM_FILES})

install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/data/i18n)
