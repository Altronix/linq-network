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

option(WITH_SQLITE "Include database support" ON)
if(WITH_SQLITE)
  function(append_sqlite_compiler_flags dst)
    list(APPEND definitions "-DWITH_SQLITE")
    list(APPEND definitions "-DSQLITE_CORE")
    list(APPEND definitions "-DSQLITE_DEFAULT_FORIEGN_KEYS=1")
    list(APPEND definitions "-DSQLITE_ENABLE_API_ARMOR")
    list(APPEND definitions "-DSQLITE_ENABLE_COLUMN_METADATA")
    list(APPEND definitions "-DSQLITE_ENABLE_DBSTAT_VTAB")
    list(APPEND definitions "-DSQLITE_ENABLE_FTS3")
    list(APPEND definitions "-DSQLITE_ENABLE_FTS3_PARENTHESIS")
    list(APPEND definitions "-DSQLITE_ENABLE_FTS5")
    list(APPEND definitions "-DSQLITE_ENABLE_JSON1")
    list(APPEND definitions "-DSQLITE_ENABLE_LOAD_EXTENSION")
    list(APPEND definitions "-DSQLITE_ENABLE_MEMORY_MANAGEMENT")
    list(APPEND definitions "-DSQLITE_ENABLE_RTREE")
    list(APPEND definitions "-DSQLITE_ENABLE_STAT2")
    list(APPEND definitions "-DSQLITE_ENABLE_STAT4")
    list(APPEND definitions "-DSQLITE_SOUNDEX")
    list(APPEND definitions "-DSQLITE_THREADSAFE=1")
    list(APPEND definitions "-DSQLITE_USE_URI")
    list(APPEND definitions "-DHAVE_USLEEP=1")
    set(${dst} ${definitions} PARENT_SCOPE)
  endfunction()

  function(append_mongoose_compiler_flags dst)
    list(APPEND definitions "-DMG_ENABLE_HTTP_STREAMING_MULTIPART")
    list(APPEND definitions "-DMG_ENABLE_WEBSOCKET")
    list(APPEND definitions "-DMG_ENABLE_CALLBACK_USERDATA")
    list(APPEND definitions "-DMG_ENABLE_SSL")
    set(${dst} ${definitions} PARENT_SCOPE)
  endfunction()
endif()

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
if(USE_SYSTEM_OPENSSL)
  find_package(OpenSSL REQUIRED)
  set(TLS_SHARED_LIBS OpenSSL::Crypto OpenSSL::SSL)
  set(TLS_STATIC_LIBS OpenSSL::Crypto OpenSSL::SSL)
else()
  include(cmake/openssl.cmake)
  set(TLS_STATIC_LIBS ssl-static crypto-static)
  set(TLS_SHARED_LIBS ssl-shared crypto-shared)
endif()

option(USE_SYSTEM_JSMN_WEB_TOKENS "Look for jwt library on target system" OFF)
if(USE_SYSTEM_JSMN_WEB_TOKENS)
  set(JSMN_WEB_TOKENS_SHARED_LIBS jsmn-web-tokens)
  set(JSMN_WEB_TOKENS_STATIC_LIBS jsmn-web-tokens)
else()
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
  add_subdirectory(test/containers)
  add_subdirectory(test/linq_network)
  add_subdirectory(test/device)
  add_subdirectory(test/devices)
  add_subdirectory(bindings/cpp/test)

  # add test coverage
  APPEND_COVERAGE_COMPILER_FLAGS()
  set(COVERAGE_EXCLUDES 
    "examples/server/*"
    "linq_network/base64.*"
    "linq_network/mongoose.*"
    "linq_network/sqlite3.*"
    "linq_network/sys/mock/*"
    "external/http_parser/*")

  setup_target_for_coverage_lcov(NAME test-coverage
    EXECUTABLE test-containers
    COMMAND test-linq-network
    COMMAND test-device
    COMMAND test-devices
    COMMAND test-http
    COMMAND test-database
    COMMAND test-routes
    COMMAND binding-linq-network-cpp-test)

endif()

add_subdirectory(linq_utils)
add_subdirectory(linq_database)
add_subdirectory(linq_network)
add_subdirectory(linq_http)

option(BUILD_EXAMPLES "Whether or not to build the examples" OFF)
if(BUILD_EXAMPLES)
  add_subdirectory(examples/server)
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
