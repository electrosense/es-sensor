# AVRO_FOUND - true if library and headers were found
# AVRO_INCLUDE_DIRS - include directories
# AVRO_LIBRARIES - library directories

find_package(PkgConfig)
pkg_check_modules(PC_AVRO QUIET json-c)

find_path(AVRO_INCLUDE_DIR avro.h
        /usr/pkgs64/include
        /usr/include
        /usr/local/include)

FIND_LIBRARY(AVRO_LIBRARY
        NAMES avro
        PATHS /usr/lib/x86_64-linux-gnu/
        /usr/pkgs64/lib
        /usr/lib64
        /usr/lib
        /usr/local/lib
        /usr/lib/arm-linux-gnueabihf/
        NO_DEFAULT_PATH
        )

IF (AVRO_LIBRARY AND AVRO_INCLUDE_DIR)
    SET(AVRO_LIBRARIES ${AVRO_LIBRARY})
    SET(AVRO_FOUND "YES")
ELSE (AVRO_LIBRARY AND AVRO_INCLUDE_DIR)
    SET(AVRO_FOUND "NO")
ENDIF (AVRO_LIBRARY AND AVRO_INCLUDE_DIR)

IF (AVRO_FOUND)
    IF (NOT AVRO_FIND_QUIETLY)
        MESSAGE(STATUS "Found AVRO: ${AVRO_LIBRARIES}")
    ENDIF (NOT AVRO_FIND_QUIETLY)
ELSE (AVRO_FOUND)
    IF (AVRO_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find AVRO library")
    ENDIF (AVRO_FIND_REQUIRED)
ENDIF (AVRO_FOUND)

MARK_AS_ADVANCED(
        AVRO_LIBRARY
        AVRO_INCLUDE_DIR
)

