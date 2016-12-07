# - Find mysqlclient
# Find the native MySQL includes and library
#
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - List of libraries when using MySQL.
#  MYSQL_FOUND       - True if MySQL found.

find_path(MYSQL_INCLUDE_DIR mysql.h
        /usr/local/include/mysql
        /usr/include/mysql
        )

set(MYSQL_NAMES mysqlclient mysqlclient_r)
find_library(MYSQL_LIBRARY
        NAMES ${MYSQL_NAMES}
        PATHS /usr/lib /usr/local/lib
        PATH_SUFFIXES mysql
        )

IF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    set(MYSQL_FOUND TRUE)
    set( MYSQL_LIBRARIES ${MYSQL_LIBRARY} )
ELSE (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    set(MYSQL_FOUND FALSE)
    set( MYSQL_LIBRARIES )
ENDIF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)

IF (MYSQL_FOUND)
    IF (NOT MYSQL_FIND_QUIETLY)
        message(STATUS "Found MySQL: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARY}")
    ENDIF (NOT MYSQL_FIND_QUIETLY)
ELSE (MYSQL_FOUND)
    IF (MYSQL_FIND_REQUIRED)
        message(STATUS "Looked for MySQL libraries named ${MYSQL_NAMES}.")
        message(FATAL_ERROR "Could NOT find MySQL library")
    ENDIF (MYSQL_FIND_REQUIRED)
ENDIF (MYSQL_FOUND)

mark_as_advanced(
        MYSQL_LIBRARY
        MYSQL_INCLUDE_DIR
)
