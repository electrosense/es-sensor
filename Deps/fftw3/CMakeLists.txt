
set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(FindPkgConfig)

find_package (FFTW3 COMPONENTS single double threads)
