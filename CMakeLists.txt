include(ExternalProject)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

cmake_minimum_required(VERSION 2.8.12)

project(fcs-genome)

find_package(Boost 1.53.0 COMPONENTS
	  system thread iostreams filesystem regex program_options REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Debug")
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

# add external projects
ExternalProject_Add(glog-download
    PREFIX "glog"
    URL https://s3.amazonaws.com/fcs-build-public/glog-falcon.tar.gz
    URL_MD5 6d58007c984c961afe3704241022da15
    SOURCE_DIR "${CMAKE_BINARY_DIR}/glog/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

ExternalProject_Add(gflags-download
    PREFIX "gflags"
    URL https://s3.amazonaws.com/fcs-build-public/gflags.tar.gz
    URL_MD5 1de8187489fbced5cc86c2ba241440e4
    SOURCE_DIR "${CMAKE_BINARY_DIR}/gflags/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

ExternalProject_Add(googletest-download
    PREFIX "googletest"
    URL https://s3.amazonaws.com/fcs-build-public/googletest.tar.gz
    URL_MD5 18fda945045354e264e3cca5428525d6
    SOURCE_DIR "${CMAKE_BINARY_DIR}/googletest/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

ExternalProject_Add(jsoncpp-download
    PREFIX "jsoncpp"
    URL https://s3.amazonaws.com/fcs-build-public/jsoncpp-1.7.7.tar.gz
    URL_MD5 10df83654beb3477ecd4082763bce311
    SOURCE_DIR "${CMAKE_BINARY_DIR}/jsoncpp/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

ExternalProject_Add(falconlm-download
    PREFIX "falconlm"
    URL https://s3.amazonaws.com/fcs-build-public/falcon-lic.tar.gz
    URL_MD5 8f36e6cb08c7cb7a677d749bee1bfe6e
    CONFIGURE_COMMAND ""
    SOURCE_DIR "${CMAKE_BINARY_DIR}/falconlm/install"
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

set(GLOG_DIR "${CMAKE_BINARY_DIR}/glog/install")
set(GFLAGS_DIR "${CMAKE_BINARY_DIR}/gflags/install")
set(GTEST_DIR "${CMAKE_BINARY_DIR}/googletest/install")
set(JSONCPP_DIR "${CMAKE_BINARY_DIR}/jsoncpp/install")
set(FALCONLM_DIR "${CMAKE_BINARY_DIR}/falconlm/install")

#ExternalProject_Get_Property(glog-download GLOG_DIR)
#ExternalProject_Get_Property(gflags-download GFLAGS_DIR)
#ExternalProject_Get_Property(googletest-download GTEST_DIR)
#ExternalProject_Get_Property(jsoncpp-download JSONCPP_DIR)
#ExternalProject_Get_Property(falconlm-download FALCONLM_DIR)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
 		${GLOG_DIR}/include
 		${GFLAGS_DIR}/include
 		${GTEST_DIR}/include
 		${JSONCPP_DIR}/include
 		${FALCONLM_DIR}/include
    ${Boost_INCLUDE_DIRS})

link_directories(
		${GLOG_DIR}/lib
		${GFLAGS_DIR}/lib
		${GTEST_DIR}/lib
		${JSONCPP_DIR}/lib
		${FALCONLM_DIR}/lib)

# find source files
file(GLOB_RECURSE SRC_LIST src/*.cpp)
list(REMOVE_ITEM SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(fcs-genome-objs ${SRC_LIST})

add_dependencies(fcs-genome-objs glog-download)
add_dependencies(fcs-genome-objs gflags-download)
add_dependencies(fcs-genome-objs googletest-download)
add_dependencies(fcs-genome-objs jsoncpp-download)
add_dependencies(fcs-genome-objs falconlm-download)

add_executable(fcs-genome ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(fcs-genome
    fcs-genome-objs
    glog
    gflags
    jsoncpp
    falcon_license
    ${CMAKE_DL_LIBS} 
    ${Boost_LIBRARIES} 
    ${ZLIB_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT})

enable_testing()
add_subdirectory(test)
