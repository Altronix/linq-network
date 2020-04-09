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

# Note buildroot sets BUILD_SHARED_LIBS
option(BUILD_SHARED "Whether or not to build the shared object"  ON)
option(BUILD_STATIC "Whether or not to build the static archive" ON)

set(LINQ_LOG_LEVEL
  "INFO"
  CACHE STRING 
  "set logging to TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NONE")
if(${LINQ_LOG_LEVEL} STREQUAL "TRACE")
  set(LINQ_LOG_LEVEL_INT 6)
elseif(${LINQ_LOG_LEVEL} STREQUAL "DEBUG")
  set(LINQ_LOG_LEVEL_INT 5)
elseif(${LINQ_LOG_LEVEL} STREQUAL "INFO")
  set(LINQ_LOG_LEVEL_INT 4)
elseif(${LINQ_LOG_LEVEL} STREQUAL "WARN")
  set(LINQ_LOG_LEVEL_INT 3)
elseif(${LINQ_LOG_LEVEL} STREQUAL "ERROR")
  set(LINQ_LOG_LEVEL_INT 2)
elseif(${LINQ_LOG_LEVEL} STREQUAL "FATAL")
  set(LINQ_LOG_LEVEL_INT 1)
else()
  set(LINQ_LOG_LEVEL_INT 0)
endif()
function(append_log_level_compiler_flags dst)
  MESSAGE(STATUS "LINQ_LOG_LEVEL set ${LINQ_LOG_LEVEL}")
  list(APPEND definitions "-DLINQ_LOG_LEVEL=${LINQ_LOG_LEVEL_INT}")
  set(${dst} ${definitions} PARENT_SCOPE)
endfunction()

include(cmake/mongoose.cmake)
include(cmake/sqlite.cmake)

option(BUILD_LINQD "Whether or not to build the daemon" ON)

option(USE_SYSTEM_ZMQ "Look for libzmq and libczmq on target system." OFF)
if(USE_SYSTEM_ZMQ)
  find_package(czmq)
  find_package(libzmq)
  set(LIBZMQ_SHARED_LIBS zmq czmq)
else()
  include(cmake/libzmq.cmake)
  include(cmake/czmq.cmake)
  set(LIBZMQ_SHARED_LIBS czmq-shared zmq-shared)
  set(LIBZMQ_STATIC_LIBS czmq-static zmq-static)
endif()

option(USE_SYSTEM_OPENSSL "Look for openssl on target system." ON)
if(BUILD_LINQD AND USE_SYSTEM_OPENSSL)
  find_package(OpenSSL REQUIRED)
  set(TLS_SHARED_LIBS OpenSSL::Crypto OpenSSL::SSL)
  set(TLS_STATIC_LIBS OpenSSL::Crypto OpenSSL::SSL)
elseif(BUILD_LINQD)
  include(cmake/openssl.cmake)
  set(TLS_STATIC_LIBS ssl-static crypto-static)
  set(TLS_SHARED_LIBS ssl-shared crypto-shared)
endif()

option(USE_SYSTEM_JSMN_WEB_TOKENS "Look for jwt library on target system" OFF)
if(BUILD_LINQD AND USE_SYSTEM_JSMN_WEB_TOKENS)
  set(JSMN_WEB_TOKENS_SHARED_LIBS jsmn-web-tokens)
  set(JSMN_WEB_TOKENS_STATIC_LIBS jsmn-web-tokens)
elseif(BUILD_LINQD)
  include(cmake/jsmn-web-tokens.cmake)
  set(JSMN_WEB_TOKENS_SHARED_LIBS ${JSMN_WEB_TOKENS_SHARED_LIBRARIES})
  set(JSMN_WEB_TOKENS_STATIC_LIBS ${JSMN_WEB_TOKENS_STATIC_LIBRARIES})
endif()

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
  add_subdirectory(test/_mocks/mock_czmq)
  add_subdirectory(test/_mocks/mock_mongoose)
  add_subdirectory(test/_mocks/mock_sqlite)
  add_subdirectory(linq_database/test)
  add_subdirectory(linq_http/test/http)
  add_subdirectory(linq_http/test/routes)
  add_subdirectory(linq_daemon/test)
  add_subdirectory(linq_utils/test/containers)
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
    COMMAND test-http
    COMMAND test-database
    COMMAND test-routes
    COMMAND binding-linq-network-cpp-test)

endif()

add_subdirectory(linq_utils)
add_subdirectory(linq_network)

if(BUILD_LINQD)
  add_subdirectory(linq_database)
  add_subdirectory(linq_http)
  add_subdirectory(linq_daemon)
endif()

option(WITH_CPP_BINDING "Build the C++ bindings" ON)
if(WITH_CPP_BINDING)
  add_subdirectory(bindings/cpp)
endif()

option(WITH_NODEJS_BINDING "Build the NODEJS bindings" OFF)
if(WITH_NODEJS_BINDING)
  message(STATUS "Building NODEJS binding")
  message(STATUS "Make sure to build NodeJS Binding by using npm install command!")
  add_subdirectory(bindings/nodejs)
endif()
