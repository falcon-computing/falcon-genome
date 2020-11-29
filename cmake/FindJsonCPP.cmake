ExternalProject_Add(jsoncpp-download
    PREFIX "jsoncpp"
    URL ${DEPS}/jsoncpp-1.7.7.tar.gz
    SOURCE_DIR "${CMAKE_BINARY_DIR}/jsoncpp/install"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

add_custom_target(JsonCPP)
add_dependencies(JsonCPP jsoncpp-download)

set(JsonCPP_DIR "${CMAKE_BINARY_DIR}/jsoncpp/install")
set(JsonCPP_INCLUDE_DIRS "${JsonCPP_DIR}/include")
set(JsonCPP_LIBRARY_DIRS "${JsonCPP_DIR}/lib")
set(JsonCPP_LIBRARIES "jsoncpp")
