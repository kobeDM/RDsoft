cmake_minimum_required(VERSION 3.1.3)
set (CMAKE_CXX_STANDARD 11)
project(analog LANGUAGES CXX)

option(BUILD_RDDAQ "Build the libm2k-dependent RD-daq executable" ON)

set(Targets Dat2Root CalcRnRate)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
list(APPEND CMAKE_MODULE_PATH $ENV{ROOTSYS}/etc/cmake)

find_package(ROOT REQUIRED)
include_directories(${ROOT_INCLUDE_DIRS})

include(${ROOT_USE_FILE})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)


if (NOT WIN32)
	find_library(PTHREAD_LIBRARIES pthread)
endif()

if (BUILD_RDDAQ)
	add_executable(RD-daq "RD-daq.cc")
	target_link_libraries(RD-daq PRIVATE m2k)
	list(APPEND Targets RD-daq)
endif()

add_executable(Dat2Root "Dat2Root.cc")
add_executable(CalcRnRate "CalcRnRate.cc")


target_link_libraries(Dat2Root PRIVATE ROOT::RIO ROOT::Net ROOT::Tree)
target_link_libraries(CalcRnRate PRIVATE ROOT::RIO ROOT::Net ROOT::Tree ROOT::Hist ROOT::Graf ROOT::Gpad)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/..)
install(TARGETS ${Targets} DESTINATION bin)
