include (Documentation)

#set (BUILD_DOXYGEN ON)
#set (DOCUMENTATION_HTML_TARZ ON)

set (DOXYFILE "Doxyfile")
set (DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)
set (DOXYGEN_STRIP_FROM_PATH ${HOLDINGNUTS_SOURCE_DIR})

set (DOXYGEN_INPUT
	"\"${HOLDINGNUTS_SOURCE_DIR}/src\" \\
	\"${HOLDINGNUTS_SOURCE_DIR}/src/libpoker\" \\
	\"${HOLDINGNUTS_SOURCE_DIR}/src/system\" \\
	\"${HOLDINGNUTS_SOURCE_DIR}/src/server\" \\
	\"${HOLDINGNUTS_SOURCE_DIR}/src/client\""
)

# fill placeholders in Doxyfile
configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/${DOXYFILE}.in
    ${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE} @ONLY IMMEDIATE
)

add_custom_target (documentation
	COMMAND ${DOXYGEN_EXECUTABLE} \"${CMAKE_CURRENT_BINARY_DIR}/${DOXYFILE}\"
	COMMENT "Building doxygen documentation"
)
