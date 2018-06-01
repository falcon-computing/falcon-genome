set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS="-fprofile-arcs -ftest-coverage") 

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

add_executable(test_app ${SRC_LIST})
add_dependencies(test_app glog-download)
add_dependencies(test_app gflags-download)
add_dependencies(test_app googletest-download)
add_dependencies(test_app jsoncpp-download)

target_link_libraries(test_app
    fcs-genome-objs
    gcov
    glog
    gflags
    gtest
    jsoncpp
    ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_test(fcs-genome-test test_app)
