# Locate SQLITE3
# This module defines
#  SQLITE3_FOUND, SQLITE3 is found
#  SQLITE3_INCLUDE_DIR, the SQLITE3 include directory
#  SQLITE3_LIBRARIES - the SQLITE3 libraries
#
# This module is part of the HoldingNuts project and is licensed under the GPLv3


if (SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)
   # in cache already, be quiet
   SET(SQLITE3_FIND_QUIETLY TRUE)
endif (SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)

FIND_PATH(SQLITE3_INCLUDE_DIR sqlite3.h
  HINTS
  $ENV{SQLITE3_DIR}
  PATH_SUFFIXES include/sqlite3 include/sqlite include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

FIND_LIBRARY(SQLITE3_LIBRARIES
  NAMES sqlite3
  HINTS
  $ENV{SQLITE3_DIR}
  PATH_SUFFIXES lib64 lib
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)


INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SQLITE3 DEFAULT_MSG SQLITE3_INCLUDE_DIR SQLITE3_LIBRARIES)

MARK_AS_ADVANCED(SQLITE3_INCLUDE_DIR SQLITE3_LIBRARIES)
