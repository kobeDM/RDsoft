cmake_minimum_required(VERSION 3.1.3)
set (CMAKE_CXX_STANDARD 11)
project(analog LANGUAGES CXX)

set(Targets RD-daq RD_dat2root RD_rnrate)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
list(APPEND CMAKE_MODULE_PATH $ENV{ROOTSYS}/etc/cmake)

find_package(ROOT REQUIRED)
include_directories(${ROOT_INCLUDE_DIRS})

include(${ROOT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


if (NOT WIN32)
	find_library(PTHREAD_LIBRARIES pthread)
endif()

#if (LIBM2K_VERSION)			# in-source build
#	set(PROJECT_VERSION ${LIBM2K_VERSION})
#else()					# standalone build
#	set(PROJECT_VERSION "1.0.0")
#	find_package(libm2k REQUIRED)
#endif()

add_executable(RD-daq "RD-daq.cpp")
add_executable(RD_dat2root "RD_dat2root.cpp")
add_executable(RD_rnrate "RD_rnrate.cpp")

target_link_libraries(RD-daq PRIVATE m2k)
target_link_libraries(RD_dat2root PRIVATE ROOT::RIO ROOT::Net ROOT::Tree)
target_link_libraries(RD_rnrate PRIVATE ROOT::RIO ROOT::Net ROOT::Tree ROOT::Hist ROOT::Graf ROOT::Gpad)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/..)
#install(TARGETS RD-daq DESTINATION bin)
install(TARGETS ${Targets} DESTINATION bin)
