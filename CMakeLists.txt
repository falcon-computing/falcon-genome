set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

cmake_minimum_required(VERSION 2.8.12)

project(fcs-genome)

find_package(Boost 1.53.0 COMPONENTS
	  system thread iostreams filesystem regex program_options REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads)

set(CMAKE_CXX_FLAGS "-std=c++0x -DBOOST_NO_CXX11_SCOPED_ENUMS -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -O2 -DNDEBUG -DUSELICENSE")
#set(CMAKE_BUILD_TYPE "Debug")

# get version str from git
execute_process(
    COMMAND git describe --tags --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)

if(DEPLOYMENT_DST) 
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DDEPLOY_${DEPLOYMENT_DST}")
    set(BIN_VERSION "${GIT_VERSION}-${DEPLOYMENT_DST}")
    message(STATUS "destination deployment platform is ${DEPLOYMENT_DST}")
else(DEPLOYMENT_DST)
    set(BIN_VERSION "${GIT_VERSION}")
endif(DEPLOYMENT_DST)

message(STATUS "version: ${BIN_VERSION}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVERSION=\\\"${BIN_VERSION}\\\"")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
		${FLM_DIR}/include
    ${GLOG_DIR}/include
    ${GTEST_DIR}/include
		${JSONCPP_DIR}/include)

link_directories(
		${GLOG_DIR}/lib
		${GTEST_DIR}/lib
		${JSONCPP_DIR}/lib
		${FLM_DIR}/lib)

#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC_LIST)
file(GLOB_RECURSE SRC_LIST src/*.cpp)
list(REMOVE_ITEM SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(fcs-genome-objs ${SRC_LIST})

add_subdirectory(deps)

add_executable(fcs-genome ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

add_dependencies(fcs-genome glog-download)
add_dependencies(fcs-genome googletest-download)
add_dependencies(fcs-genome jsoncpp-download)
add_dependencies(fcs-genome falcon-lic-download)

target_link_libraries(fcs-genome
    fcs-genome-objs
    glog
    jsoncpp
    falcon_license
    ${CMAKE_DL_LIBS} 
    ${Boost_LIBRARIES} 
    ${ZLIB_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

enable_testing()
add_subdirectory(test)
