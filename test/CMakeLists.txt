set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS="-fprofile-arcs -ftest-coverage") 

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

add_executable(test_app ${SRC_LIST})

# this is required to force downloading all 
# packages before the compilation starts
add_dependencies(test_app JsonCPP Google)

target_link_libraries(test_app
    fcs-genome-objs
    gcov
    ${Google_LIBRARIES}
    ${JsonCPP_LIBRARIES}
    ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resource DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_test(fcs-genome-test test_app)
