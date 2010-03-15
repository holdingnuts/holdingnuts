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
		ADD_CUSTOM_COMMAND( OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/pclient_res.obj
			COMMAND ${WINDRES}
			-I${CMAKE_CURRENT_SOURCE_DIR}
			-o ${CMAKE_CURRENT_BINARY_DIR}/pclient_res.obj
			< ${CMAKE_CURRENT_SOURCE_DIR}/pclient.rc
		)
		
		LIST(APPEND aux_obj pclient_res.obj)
	ELSE (MINGW)
		# FIXME: use VC resource compiler rc.exe
	ENDIF (MINGW)
ENDIF (WIN32)

# FIXME: Which one is the correct define for Haiku?
IF (BEOS)
       LIST(APPEND aux_lib network)
ENDIF (BEOS)


# use SDL only for audio
if (ENABLE_AUDIO)
	Find_Package ( SDL REQUIRED )

	include_directories ( ${SDL_INCLUDE_DIR} )
	
	LIST (APPEND aux_src Audio.c)
	LIST (APPEND aux_lib ${SDL_LIBRARY})
else (ENABLE_AUDIO)
	add_definitions (-DNOAUDIO=1)
endif (ENABLE_AUDIO)

SET( QT_USE_QTNETWORK true )

Find_Package ( Qt4 REQUIRED )

# http://www.qtforum.org/article/26305/problem-running-configure-to-install-qt.html

INCLUDE(${QT_USE_FILE})


SET(QtApp_RCCS pclient.qrc)

QT4_ADD_RESOURCES(QtApp_RCC_SRCS ${QtApp_RCCS})

# Warning: older CMake versions (2.6-patch-0) do not provide moc-qt4 with preprocessor definitions
QT4_WRAP_CPP (
	sources_MOC
	pclient.hpp WMain.hpp SettingsDialog.hpp CreateGameDialog.hpp
	AboutDialog.hpp GameListTableModel.hpp PlayerListTableModel.hpp GameListSortFilterProxyModel.hpp PlayerListSortFilterProxyModel.hpp
	WTable.hpp ChatBox.hpp DealerButton.hpp Seat.hpp EditableSlider.hpp TimeOut.hpp
	Chip.hpp ChipStack.hpp
)

add_executable (
	holdingnuts
	WIN32    # when building for windows: do not show the console window
	pclient.cpp ${aux_obj}
	WMain.cpp SettingsDialog.cpp CreateGameDialog.cpp AboutDialog.cpp
	GameListTableModel.cpp PlayerListTableModel.cpp GameListSortFilterProxyModel.cpp PlayerListSortFilterProxyModel.cpp
	WTable.cpp ChatBox.cpp DealerButton.cpp Seat.cpp SeatImages.cpp EditableSlider.cpp TimeOut.cpp
	Chip.cpp ChipStack.cpp
	${aux_src}
	${sources_MOC} ${QtApp_RCC_SRCS}
)

target_link_libraries (
   holdingnuts
   Poker SysAccess System
   ${QT_LIBRARIES}
   ${aux_lib}
)

INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/holdingnuts DESTINATION
	${CMAKE_INSTALL_PREFIX}/bin)

############################################################

# data-directory
INSTALL(DIRECTORY ${HOLDINGNUTS_SOURCE_DIR}/data DESTINATION
	${CMAKE_DATA_PATH}/holdingnuts)

############################################################

# translations

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

add_custom_target (translations ALL DEPENDS ${QM_FILES})

install(FILES ${QM_FILES} DESTINATION	
	${CMAKE_DATA_PATH}/holdingnuts/data/i18n)
