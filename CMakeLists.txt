cmake_minimum_required(VERSION 3.9.3...3.16)
project(linq-network)

find_package(Git)
if(NOT Git_FOUND)
  message(FATAL_ERROR "Git not found! Git is required to install submodules")
endif()

# dependency install
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
find_package(Threads)

# Compiler stuff
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE on)

include(cmake/helpers.cmake)
include(cmake/config.cmake)
include(cmake/mongoose.cmake)
include(cmake/sqlite.cmake)

# [DEPENDENCIES] libzmq czmq jsmn-web-tokens openssl
if(BUILD_DEPENDENCIES)
  if(BUILD_LINQD)
    find_package(OpenSSL REQUIRED)
    set(TLS_LIBS OpenSSL::Crypto OpenSSL::SSL)
    include(cmake/jsmn-web-tokens.cmake)
    set(JSMN_WEB_TOKENS_LIBS jsmn-web-tokens-static)
  endif()
  include(cmake/libzmq.cmake)
  include(cmake/czmq.cmake)
  set(ZMQ_LIBS czmq-static zmq-static)
else()
  if(BUILD_LINQD)
    find_package(OpenSSL REQUIRED)
    find_package(jsmn-web-tokens REQUIRED)
    set(TLS_LIBS OpenSSL::Crypto OpenSSL::SSL)
    set(JSMN_WEB_TOKENS_LIBS jsmn-web-tokens)
  endif()
  find_package(czmq REQUIRED)
  find_package(libzmq REQUIRED)
  set(ZMQ_LIBS zmq czmq)
endif()

# if(USE_SYSTEM_ZMQ)
#   find_package(czmq)
#   find_package(libzmq)
#   set(ZMQ_LIBS zmq czmq)
# else()
#   include(cmake/libzmq.cmake)
#   include(cmake/czmq.cmake)
#   set(ZMQ_LIBS czmq-static zmq-static)
# endif()
# 
# if(BUILD_LINQD AND USE_SYSTEM_OPENSSL)
#   find_package(OpenSSL REQUIRED)
#   set(TLS_LIBS OpenSSL::Crypto OpenSSL::SSL)
# elseif(BUILD_LINQD)
#   include(cmake/openssl.cmake)
#   set(TLS_LIBS ssl-static crypto-static)
# endif()
# 
# if(BUILD_LINQD AND USE_SYSTEM_JSMN_WEB_TOKENS)
#   find_package(jsmn-web-tokens REQUIRED)
#   set(JSMN_WEB_TOKENS_LIBS jsmn-web-tokens)
# elseif(BUILD_LINQD)
#   include(cmake/jsmn-web-tokens.cmake)
#   set(JSMN_WEB_TOKENS_LIBS jsmn-web-tokens-static)
# endif()

if(ENABLE_TESTING)
  set(CMAKE_BUILD_TYPE Debug)
  set(CMAKE_EXPORT_COMPILE_COMMANDS on)
  enable_testing()
  include(CTest)
  include(cmake/cmocka.cmake)
  include(cmake/CodeCoverage.cmake)
  include(cmake/http-parser.cmake)
  if(EXISTS "${CMAKE_SOURCE_DIR}/external/linq-common/CMakeLists.txt")
    MESSAGE(STATUS "linq-common found!")
    MESSAGE(STATUS "Building linq-common integration test")
    include(cmake/wolfssl.cmake)
    include(cmake/test-fixture.cmake)
    add_subdirectory(test/integration)
    add_subdirectory(test/_fixture)
  endif()
  add_subdirectory(test/_helpers)
  add_subdirectory(test/_mocks/mock_utils)
  add_subdirectory(test/_mocks/mock_file)
  add_subdirectory(test/_mocks/mock_czmq)
  add_subdirectory(test/_mocks/mock_mongoose)
  add_subdirectory(test/_mocks/mock_sqlite)
  add_subdirectory(linq_database/test)
  add_subdirectory(linq_http/test/http)
  add_subdirectory(linq_http/test/routes)
  add_subdirectory(linq_daemon/test)
  add_subdirectory(linq_usb/device/test)
  add_subdirectory(linq_utils/test/containers)
  add_subdirectory(linq_utils/test/jsmn)
  add_subdirectory(linq_utils/test/sys)
  add_subdirectory(linq_network/test/linq_network)
  add_subdirectory(linq_network/test/device)
  add_subdirectory(bindings/cpp/test)

  # add test coverage
  APPEND_COVERAGE_COMPILER_FLAGS()
  set(COVERAGE_EXCLUDES 
    "examples/server/*"
    "linq_http/mongoose.*"
    "linq_database/sqlite3.*"
    "linq_utils/sys/mock/*"
    "linq_utils/base64.*"
    "external/http_parser/*")

  setup_target_for_coverage_lcov(NAME test-coverage
    EXECUTABLE test-containers
    COMMAND test-linq-network
    COMMAND test-device
    COMMAND test-jsmn
    COMMAND test-http
    COMMAND test-database
    COMMAND test-routes
    COMMAND test-linqd
    COMMAND binding-linq-network-cpp-test)

endif()

add_subdirectory(linq_utils)
add_subdirectory(linq_network)

if(BUILD_LINQD)
  add_subdirectory(linq_database)
  add_subdirectory(linq_http)
  add_subdirectory(linq_daemon)
  add_subdirectory(apps/linqd)
endif()

if(BUILD_USBD)
  add_subdirectory(linq_usb/device)
  add_subdirectory(apps/usbd)
endif()

if(BUILD_USBH)
  add_subdirectory(linq_usb/host)
  add_subdirectory(apps/usbh)
endif()

if(WITH_CPP_BINDING)
  add_subdirectory(bindings/cpp)
endif()

if(WITH_NODEJS_BINDING)
  message(STATUS "Building NODEJS binding")
  message(STATUS "Make sure to build NodeJS Binding by using npm install command!")
  add_subdirectory(bindings/nodejs)
endif()
