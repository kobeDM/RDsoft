cmake_minimum_required(VERSION 3.1.3)
set (CMAKE_CXX_STANDARD 11)
project(analog LANGUAGES CXX)


if (NOT WIN32)
	find_library(PTHREAD_LIBRARIES pthread)
endif()

if (LIBM2K_VERSION)			# in-source build
	set(PROJECT_VERSION ${LIBM2K_VERSION})
else()					# standalone build
	set(PROJECT_VERSION "1.0.0")
	find_package(libm2k REQUIRED)
endif()

add_executable(RD-daq "RD-daq.cpp")

target_link_libraries(RD-daq PRIVATE libm2k::libm2k)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/..)
install(TARGETS RD-daq DESTINATION bin)



