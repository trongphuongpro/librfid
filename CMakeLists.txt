cmake_minimum_required(VERSION 3.0)

set(TARGET mfrc522)

#-----------------------------------------------------------------------------#

if (SERIES STREQUAL ATMEGA)
	# temporarily
	add_subdirectory($ENV{HOME}/Work/BareMetal/libraries/uart uart)
	add_library(${TARGET} STATIC src/mfrc522_atmega.c
							lib/spi_atmega.c)

elseif (SERIES STREQUAL TIVA)
	add_library(${TARGET} STATIC src/mfrc522_tiva.c
							lib/spi_tiva.c
							# temporarily
							${TIVAWARE_PATH}/utils/uartstdio.c)

else()
	message(">> Failure due to missing SERIES.")

endif()

target_include_directories(${TARGET} PRIVATE include)

#-----------------------------------------------------------------------------#

if (SERIES STREQUAL ATMEGA)
	# temporarily
	target_link_libraries(${TARGET} uart)

	target_compile_options(${TARGET} PUBLIC -mmcu=${MCU}
											-std=gnu11
											-O2
											-Wall
											-Werror
	)

	target_compile_definitions(${TARGET} PUBLIC F_CPU=${F_CPU})

#-----------------------------------------------------------------------------#

elseif (SERIES STREQUAL TIVA)
	target_link_libraries(${TARGET} ${TIVAWARE_PATH}/driverlib/gcc/libdriver.a)
	target_include_directories(${TARGET} PRIVATE ${TIVAWARE_PATH})

	target_compile_options(${TARGET} PUBLIC -mthumb
											${CPU}
											${FPU}
											${FLOAT}
											-ffunction-sections 
											-fdata-sections    
											-MD                                         
											-pedantic           
											-std=gnu11
											-O2
											-Wall
											-Werror
	)

	target_compile_definitions(${TARGET} PUBLIC TARGET_IS_${REV}
												PART_${MCU}
	)

#-----------------------------------------------------------------------------#

else()
	message(">> Failure due to missing SERIES.")

endif()