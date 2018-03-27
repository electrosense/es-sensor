cmake_minimum_required(VERSION 3.7)
project(es_sensor)

# Set version
SET (VERSION 2.0.0)
add_definitions( -DAPP_VERSION=${VERSION})


SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++11")

# Add the local Find*.cmake scripts
LIST (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

FIND_PACKAGE( RTLSDR REQUIRED )
INCLUDE_DIRECTORIES("${RTLSDR_INCLUDE_DIR}")

# liquid, avro, ssl, crypto


configure_file( version_config.h.in generated/version_config.h )


set(SOURCE_FILES main.cpp drivers/Driver.h drivers/rtlsdr/rtlsdrDriver.cpp drivers/rtlsdr/rtlsdrDriver.h
        context/ElectrosenseContext.cpp context/ElectrosenseContext.h
        drivers/common/SequentialHopping.cpp drivers/common/SequentialHopping.h
        types/SpectrumSegment.cpp types/SpectrumSegment.h ProcessingBlocks/RemoveDC.cpp ProcessingBlocks/RemoveDC.h
        ProcessingBlocks/Windowing.cpp ProcessingBlocks/Windowing.h ProcessingBlocks/FFT.cpp ProcessingBlocks/FFT.h ProcessingBlocks/Averaging.cpp ProcessingBlocks/Averaging.h MiscBlocks/FileSink.cpp MiscBlocks/FileSink.h MiscBlocks/AvroSerialization.cpp MiscBlocks/AvroSerialization.h MiscBlocks/Transmission.cpp MiscBlocks/Transmission.h misc/TLS.cpp misc/TLS.h misc/TCP.cpp misc/TCP.h)

add_executable(es_sensor ${SOURCE_FILES})

target_link_libraries(es_sensor
        ${RTLSDR_LIBRARY}
        pthread
        dl
        liquid
        avro
        ssl
        crypto
)