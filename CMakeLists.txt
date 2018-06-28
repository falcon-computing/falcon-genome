include(ExternalProject)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

cmake_minimum_required(VERSION 2.8.12)

project(fcs-genome)

find_package(Boost 1.53.0 COMPONENTS
	  system thread iostreams filesystem regex program_options REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Google REQUIRED)
find_package(JsonCPP REQUIRED)
find_package(FalconLM REQUIRED)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# get version str from git
execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -DBOOST_NO_CXX11_SCOPED_ENUMS -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -DUSELICENSE")

if(DEPLOYMENT_DST) 
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEPLOY_${DEPLOYMENT_DST}")
    set(BIN_VERSION "${GIT_VERSION}-${DEPLOYMENT_DST}")
    message(STATUS "destination deployment platform is ${DEPLOYMENT_DST}")
else()
    set(BIN_VERSION "${GIT_VERSION}")
endif()

if (${CMAKE_BUILD_TYPE} MATCHES "Debug")
    set(BIN_VERSION "${BIN_VERSION}-dev")
endif()

message(STATUS "version: ${BIN_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVERSION=\\\"${BIN_VERSION}\\\"")

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "Install prefix is set to ~/.falcon-genome/fcs-genome/${BIN_VERSION}")
    set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.falcon-genome/fcs-genome/${BIN_VERSION}")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Google_INCLUDE_DIRS}
    ${JsonCPP_INCLUDE_DIRS}
    ${FalconLM_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS})

link_directories(
    ${Google_LIBRARY_DIRS}
    ${JsonCPP_LIBRARY_DIRS}
    ${FalconLM_LIBRARY_DIRS}
    ${Boost_LIBRARY_DIRS})

# find source files
file(GLOB_RECURSE SRC_LIST src/*.cpp)
list(REMOVE_ITEM SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(fcs-genome-objs ${SRC_LIST})

# this is required to force downloading all 
# packages before the compilation starts
add_dependencies(fcs-genome-objs Google JsonCPP FalconLM)

add_executable(fcs-genome ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

install(TARGETS fcs-genome
    RUNTIME DESTINATION .)

target_link_libraries(fcs-genome
    fcs-genome-objs
    ${Google_LIBRARIES}
    ${JsonCPP_LIBRARIES}
    ${FalconLM_LIBRARIES}
    ${CMAKE_DL_LIBS} 
    ${Boost_LIBRARIES} 
    ${ZLIB_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

enable_testing()
add_subdirectory(test)
