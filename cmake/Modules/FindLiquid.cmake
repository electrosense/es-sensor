# - Find LIQUID
# Find the native LIQUID includes and library
#
#  LIQUID_INCLUDES    - where to find LIQUID.h
#  LIQUID_LIBRARIES   - List of libraries when using LIQUID.
#  LIQUID_FOUND       - True if LIQUID found.

if (LIQUID_INCLUDES)
  # Already in cache, be silent
  set (LIQUID_FIND_QUIETLY TRUE)
endif (LIQUID_INCLUDES)

find_path (LIQUID_INCLUDES liquid/liquid.h)

find_library (LIQUID_LIBRARIES
        NAMES liquid
        PATH  /usr/lib/x86_64-linux-gnu/
        /usr/pkgs64/lib
        /usr/lib64
        /usr/lib
        /usr/local/lib
        /usr/lib/arm-linux-gnueabihf/)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(LIQUID DEFAULT_MSG LIQUID_LIBRARIES LIQUID_INCLUDES)

mark_as_advanced(LIQUID_LIBRARIES LIQUID_INCLUDE)

