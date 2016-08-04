project (electrosense)

cmake_minimum_required(VERSION 2.8)
SET( DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Deps)

# Determine current architecture
macro(dpkg_arch VAR_NAME)
        find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
        if (DPKG_PROGRAM)
          execute_process(
            COMMAND ${DPKG_PROGRAM} --print-architecture
            OUTPUT_VARIABLE ${VAR_NAME}
            OUTPUT_STRIP_TRAILING_WHITESPACE
          )
        endif(DPKG_PROGRAM)
endmacro(dpkg_arch)

# Determine number of cores
macro(num_cores VAR_NAME)
        execute_process ( COMMAND nproc OUTPUT_VARIABLE ${VAR_NAME} )
endmacro(num_cores)

dpkg_arch(ARCHITECTURE)
num_cores(NUM_CORES)
SET(NUM_CORES 4)
#message(STATUS "Architecture:  ${ARCHITECTURE}")
#message(STATUS "Cores: ${NUM_CORES}")

if ( ${ARCHITECTURE} MATCHES "armhf" AND ${NUM_CORES} EQUAL 4)
        message(STATUS "RBPi model 2")
        add_definitions(-DRBP_2)
        set( FFT_INCLUDE_FOLDER "gpu_fft_3/" )
elseif ( ${ARCHITECTURE} MATCHES "armhf" AND ${NUM_CORES} EQUAL 1)
        message(STATUS "RBPi model 1")
        add_definitions(-DRBP_1)
        set( FFT_INCLUDE_FOLDER "gpu_fft_2/" )
elseif (${ARCHITECTURE} MATCHES "amd64")
        message(STATUS "amd64 architecture")
        set( FFT_INCLUDE_FOLDER "gpu_fft_2/" )
else ( ${ARCHITECTURE} MATCHES "armhf" AND ${NUM_CORES} EQUAL 4)
        message(WARNING "architecture unknown!")
        set( FFT_INCLUDE_FOLDER "gpu_fft_2/" )
endif ( ${ARCHITECTURE} MATCHES "armhf" AND ${NUM_CORES} EQUAL 4)

message(STATUS "FFT_INCLUDE_FOLDER: ${FFT_INCLUDE_FOLDER}")


include(${DEPS_DIR}/rtl-sdr/CMakeLists.txt)
include(${DEPS_DIR}/pthread/CMakeLists.txt)
include(${DEPS_DIR}/zlib/CMakeLists.txt)
include(${DEPS_DIR}/fftw3/CMakeLists.txt)
include(${DEPS_DIR}/ssl/CMakeLists.txt)
#include(${DEPS_DIR}/kafka/CMakeLists.txt)
include(${DEPS_DIR}/avro/CMakeLists.txt)
include(${DEPS_DIR}/usb/CMakeLists.txt)
include(${DEPS_DIR}/mysql/CMakeLists.txt)
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/src/)


#INSTALL

#INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/run_collector DESTINATION /usr/bin/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/run_cpu_sensor DESTINATION /usr/bin/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/run_gpu_sensor DESTINATION /usr/bin/)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/certs     DESTINATION /etc/electrosense/)
INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/schema     DESTINATION /etc/electrosense/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/run.sh     DESTINATION /etc/electrosense/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/electrosense DESTINATION /etc/init.d/)

# Determine current architecture
macro(dpkg_arch VAR_NAME)
        find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
        if (DPKG_PROGRAM)
          execute_process(
            COMMAND ${DPKG_PROGRAM} --print-architecture
            OUTPUT_VARIABLE ${VAR_NAME}
            OUTPUT_STRIP_TRAILING_WHITESPACE
          )
        endif(DPKG_PROGRAM)
endmacro(dpkg_arch)


# Debian Package

set (CPACK_DEBIAN_PACKAGE_DEPENDS "mysql-client")
 
include (InstallRequiredSystemLibraries)
SET (CPACK_GENERATOR "DEB")
SET (CPACK_SOURCE_GENERATOR TGZ)
SET (CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
SET (CPACK_SET_DESTDIR "on")
SET (CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")

SET (VERSION 1.1.2-2)
# CPack version numbers for release tarball name.
SET (CPACK_PACKAGE_VERSION_MAJOR 0)
SET (CPACK_PACKAGE_VERSION_MINOR 0)
SET (CPACK_PACKAGE_VERSION_PATCH 2)
SET (CPACK_DEBIAN_PACKAGE_VERSION ${VERSION})

SET (CPACK_DEBIAN_PACKAGE_PRIORITY "extra")
SET (CPACK_DEBIAN_PACKAGE_SECTION "net")
dpkg_arch(CPACK_DEBIAN_PACKAGE_ARCHITECTURE)

#MESSAGE(STATUS "Dependencies: ${DEPS}")
#SET(CPACK_DEBIAN_PACKAGE_DEPENDS "${DEPS}")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/postinst"
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/postrm")

SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Electrosense is a software-defined radio to distributed spectrum monitoring.")
SET (CPACK_PACKAGE_DESCRIPTION "Electrosense is a software-defined radio to distributed spectrum monitoring.")

SET (CPACK_PACKAGE_CONTACT "Roberto Calvo <roberto.calvo@imdea.org>")
SET (CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

SET (CPACK_COMPONENTS_ALL Libraries ApplicationData)
include (CPack Documentation)

