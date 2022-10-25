cmake_minimum_required(VERSION 3.7)
project(openrfsense-sensor)

# Set version
SET (VERSION "1.0.0")
SET (DEBVERSION "1")
add_definitions( -DAPP_VERSION="${VERSION}-${DEBVERSION}")


string(TIMESTAMP TIME_COMPILATION  "%Y-%m-%d %H:%M")	

# SET(VERSION_COMPILATION "${VERSION} - ${TIME_COMPILATION}")
# message("${VERSION_COMPILATION}")


SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -std=gnu++11 -O3 ")

# Add the local Find*.cmake scripts
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

FIND_PACKAGE( RTLSDR REQUIRED )
INCLUDE_DIRECTORIES("${RTLSDR_INCLUDE_DIR}")

FIND_PACKAGE( OpenSSL REQUIRED )
INCLUDE_DIRECTORIES("OPENSSL_INCLUDE_DIR}")

FIND_PACKAGE(libusb-1.0 REQUIRED)

FIND_PACKAGE (Liquid REQUIRED)
FIND_PACKAGE (JSON-C REQUIRED)
FIND_PACKAGE (crypto REQUIRED)
FIND_PACKAGE (Avro REQUIRED)
FIND_PACKAGE (ZLIB REQUIRED)
FIND_PACKAGE (LibLZMA REQUIRED)

configure_file(version_config.h.in ${CMAKE_CURRENT_LIST_DIR}/generated/version_config.h)

# ADD_SUBDIRECTORY(drivers/rtlsdr/converter/)

set(SOURCE_FILES main.cpp drivers/Driver.h drivers/rtlsdr/rtlsdrDriver.cpp drivers/rtlsdr/rtlsdrDriver.h
    drivers/rtlsdr/converter/converter.c drivers/rtlsdr/converter/converter.h
    context/OpenRFSenseContext.cpp context/OpenRFSenseContext.h
    drivers/common/SequentialHopping.cpp drivers/common/SequentialHopping.h
    types/SpectrumSegment.cpp types/SpectrumSegment.h ProcessingBlocks/RemoveDC.cpp ProcessingBlocks/RemoveDC.h
    ProcessingBlocks/RemoveDCRTL.cpp 
    ProcessingBlocks/Windowing.cpp ProcessingBlocks/Windowing.h ProcessingBlocks/FFT.cpp ProcessingBlocks/FFT.h
    ProcessingBlocks/Averaging.cpp ProcessingBlocks/Averaging.h MiscBlocks/FileSink.cpp MiscBlocks/FileSink.h
    MiscBlocks/AvroSerialization.cpp MiscBlocks/AvroSerialization.h MiscBlocks/Transmission.cpp MiscBlocks/Transmission.h
    misc/TLS.cpp misc/TLS.h misc/TCP.cpp misc/TCP.h MiscBlocks/IQSink.cpp MiscBlocks/IQSink.h ProcessingBlocks/PSDFast.cpp
    ProcessingBlocks/PSDFast.h MiscBlocks/IQStream.cpp MiscBlocks/IQStream.h
    MiscBlocks/Filter/FIR/FIRDesign.cpp MiscBlocks/Filter/Resampler/Downsampler.cpp)

add_executable(orfs_sensor ${SOURCE_FILES})

target_link_libraries(orfs_sensor
    pthread
    dl
    ${RTLSDR_LIBRARY}
    ${LIBUSB_1_LIBRARIES}
    ${LIQUID_LIBRARIES}
    ${AVRO_LIBRARY}
    ${ZLIB_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CRYPTO_LIBRARIES}
    ${JSON-C_LIBRARIES}
)


INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/orfs_sensor DESTINATION /usr/share/openrfsense/bin/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/schemas/rtl-spec.avsc DESTINATION /usr/share/openrfsense/schemas/)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/schemas/iq-spec.avsc DESTINATION /usr/share/openrfsense/schemas/)


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

# set (CPACK_DEBIAN_PACKAGE_DEPENDS "libfftw3-3, zlib1g, liblzma5, libusb-1.0-0")

include (InstallRequiredSystemLibraries)
SET (CPACK_GENERATOR "DEB")
SET (CPACK_SOURCE_GENERATOR TGZ)
SET (CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
SET (CPACK_SET_DESTDIR "on")
SET (CPACK_PACKAGING_INSTALL_PREFIX "/usr/share/")


SET (PKG_VERSION ${VERSION}-${DEBVERSION})
# CPack version numbers for release tarball name.
SET (CPACK_PACKAGE_VERSION_MAJOR 0)
SET (CPACK_PACKAGE_VERSION_MINOR 0)
SET (CPACK_PACKAGE_VERSION_PATCH 2)
SET (CPACK_DEBIAN_PACKAGE_VERSION ${PKG_VERSION})


SET (CPACK_DEBIAN_PACKAGE_PRIORITY "extra")
SET (CPACK_DEBIAN_PACKAGE_SECTION "net")
dpkg_arch(CPACK_DEBIAN_PACKAGE_ARCHITECTURE)


set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
	"${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/postinst"
	"${CMAKE_CURRENT_SOURCE_DIR}/scripts/cmake/postrm")

SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sensor manager for OpenRFSense-enabled sensors")
SET (CPACK_PACKAGE_DESCRIPTION "OpenRFSense is an experimental software suite to explore the RF speectrum using remote devices and a centralized server.")

SET (CPACK_PACKAGE_CONTACT "Leonardo Baldin")
SET (CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${PKG_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
SET (CPACK_DEBIAN_PACKAGE_PROVIDES openrfsense-run)

SET (CPACK_COMPONENTS_ALL Libraries ApplicationData)
include (CPack Documentation)
