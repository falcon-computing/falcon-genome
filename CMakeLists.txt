include(ExternalProject)

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

# add external projects
ExternalProject_Add(
    glog-download
    PREFIX "glog"
    URL https://s3.amazonaws.com/fcs-build-public/glog-falcon.tar.gz
    URL_MD5 71f9ccd1924190356e1635ee6d6e1314
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/glog/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

ExternalProject_Add(
    gflags-download
    PREFIX "gflags"
    URL https://s3.amazonaws.com/fcs-build-public/gflags.tar.gz
    URL_MD5 1de8187489fbced5cc86c2ba241440e4
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/gflags/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

ExternalProject_Add(
    googletest-download
    PREFIX "googletest"
    URL https://s3.amazonaws.com/fcs-build-public/googletest.tar.gz
    URL_MD5 18fda945045354e264e3cca5428525d6
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/googletest/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

ExternalProject_Add(
    jsoncpp-download
    PREFIX "jsoncpp"
    URL https://s3.amazonaws.com/fcs-build-public/jsoncpp-1.7.7.tar.gz
    URL_MD5 10df83654beb3477ecd4082763bce311
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/jsoncpp/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

ExternalProject_Add(
    falcon-lic-download
    PREFIX "falcon-lic"
    URL https://s3.amazonaws.com/fcs-build-public/falcon-lic.tar.gz
    URL_MD5 8f36e6cb08c7cb7a677d749bee1bfe6e
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/falcon-lic/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

set(GLOG_DIR "${CMAKE_CURRENT_BINARY_DIR}/glog/install")
set(GFLAGS_DIR "${CMAKE_CURRENT_BINARY_DIR}/gflags/install")
set(GTEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/googletest/install")
set(JSONCPP_DIR "${CMAKE_CURRENT_BINARY_DIR}/jsoncpp/install")
set(FLM_DIR "${CMAKE_CURRENT_BINARY_DIR}/falcon-lic/install")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Boost_INCLUDE_DIRS}
		${FLM_DIR}/include
    ${GLOG_DIR}/include
    ${GTEST_DIR}/include
		${JSONCPP_DIR}/include)

link_directories(
		${GLOG_DIR}/lib
		${GFLAGS_DIR}/lib
		${GTEST_DIR}/lib
		${JSONCPP_DIR}/lib
		${FLM_DIR}/lib)

# find source files
file(GLOB_RECURSE SRC_LIST src/*.cpp)
list(REMOVE_ITEM SRC_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(fcs-genome-objs ${SRC_LIST})

add_dependencies(fcs-genome-objs glog-download)
add_dependencies(fcs-genome-objs googletest-download)
add_dependencies(fcs-genome-objs jsoncpp-download)
add_dependencies(fcs-genome-objs falcon-lic-download)

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
