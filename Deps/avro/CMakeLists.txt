
include(FindPkgConfig)


if(NOT LIBAVRO_FOUND)
  pkg_check_modules (LIBAVRO_PKG libavro)
  find_path(LIBAVRO_INCLUDE_DIRS NAMES avro.h
    PATHS
    ${LIBAVRO_PKG_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
  )

  find_library(LIBAVRO_LIBRARIES NAMES avro
    PATHS
    ${LIBAVRO_PKG_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
  )

list(APPEND DEPS libavro)

if(LIBAVRO_INCLUDE_DIRS AND LIBAVRO_LIBRARIES)
  set(LIBAVRO_FOUND TRUE CACHE INTERNAL "libavro found")
  message(STATUS "Found libavro: ${LIBAVRO_INCLUDE_DIRS}, ${LIBAVRO_LIBRARIES}")
else(LIBAVRO_INCLUDE_DIRS AND LIBAVRO_LIBRARIES)
  set(LIBAVRO_FOUND FALSE CACHE INTERNAL "libavro found")
  message(STATUS "libavro not found.")
endif(LIBAVRO_INCLUDE_DIRS AND LIBAVRO_LIBRARIES)

mark_as_advanced(LIBAVRO_LIBRARIES LIBAVRO_INCLUDE_DIRS)

endif(NOT LIBAVRO_FOUND)
