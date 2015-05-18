SET( COLLECTOR_SOURCE_FILES collector/Collector.c UTI.c ITE.c QUE.c TCP.c TLS.c THR.c)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${LIBRTLSDR_INCLUDE_DIRS}
)

add_executable (run_collector ${COLLECTOR_SOURCE_FILES})

TARGET_LINK_LIBRARIES(run_collector
	${CMAKE_THREAD_LIBS_INIT}	
	${LIBRTLSDR_LIBRARIES}		
	${ZLIB_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${KAFKA_LIBRARIES}
	)





SET ( CPU_SOURCE_FILES  sensor/Sensor.c UTI.c ITE.c QUE.c TCP.c TLS.c THR.c SDR.c FFT.c)

add_executable (run_cpu_sensor ${COLLECTOR_SOURCE_FILES})
SET_TARGET_PROPERTIES(run_cpu_sensor PROPERTIES COMPILE_FLAGS -DRPI_CPU)

TARGET_LINK_LIBRARIES(run_cpu_sensor
	${CMAKE_THREAD_LIBS_INIT}	
	${LIBRTLSDR_LIBRARIES}		
	${ZLIB_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${KAFKA_LIBRARIES}
	)


SET (GPU_SOURCE_FILES sensor/Sensor.c  FFT.c  gpu_fft_base.c  gpu_fft.c  gpu_fft_shaders.c  gpu_fft_trans.c  gpu_fft_twiddles.c  ITE.c  mailbox.c  QUE.c  SDR.c  TCP.c  THR.c  TLS.c  UTI.c)


add_executable (run_gpu_sensor ${COLLECTOR_SOURCE_FILES})
SET_TARGET_PROPERTIES(run_gpu_sensor PROPERTIES COMPILE_FLAGS -DRPI_GPU)

TARGET_LINK_LIBRARIES(run_gpu_sensor
	${CMAKE_THREAD_LIBS_INIT}	
	${LIBRTLSDR_LIBRARIES}		
	${ZLIB_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${KAFKA_LIBRARIES}
	)



