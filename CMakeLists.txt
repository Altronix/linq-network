cmake_minimum_required(VERSION 3.11)
project(linq-netw)

# dependency install
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
find_package(Threads)

# Compiler stuff
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
    set(${dst} ${definitions} PARENT_SCOPE)
  endfunction()
endif()

option(BUILD_DEPENDENCIES "Whether or not to build the tests" ON)
set(LINQ_BUILD_DEPENDENCIES
    ${BUILD_DEPENDENCIES}
    CACHE BOOL "Build the external directory for linq-netw")
if(LINQ_BUILD_DEPENDENCIES)
  include(cmake/libzmq.cmake)
  include(cmake/czmq.cmake)
  include(cmake/klib.cmake)
  include(cmake/jsmn.cmake)
else()
  find_package(czmq)
  find_package(libzmq)
  find_package(klib)
  find_package(jsmn)
  find_package(mongoose)
endif()

if(ENABLE_TESTING)
  set(CMAKE_BUILD_TYPE Debug)
  enable_testing()
  include(CTest)
  include(cmake/cmocka.cmake)
  include(cmake/CodeCoverage.cmake)
  include(cmake/http-parser.cmake)
  if(EXISTS "${CMAKE_SOURCE_DIR}/external/linq-common/CMakeLists.txt")
    MESSAGE(STATUS "linq-common found!")
    MESSAGE(STATUS "Building linq-common integration test")
    include(cmake/test-fixture.cmake)
    add_subdirectory(test/integration)
    add_subdirectory(test/_fixture)
  endif()
  add_subdirectory(test/_helpers)
  add_subdirectory(test/_mocks/mock_czmq)
  add_subdirectory(test/_mocks/mock_mongoose)
  if(WITH_SQLITE)
    add_subdirectory(test/_mocks/mock_sqlite)
    add_subdirectory(test/db)
  endif()
  add_subdirectory(test/containers)
  add_subdirectory(test/linq-netw)
  add_subdirectory(test/device)
  add_subdirectory(test/devices)
  add_subdirectory(test/http)
  add_subdirectory(test/jsmn)
  add_subdirectory(bindings/cpp/test)

  # add test coverage
  APPEND_COVERAGE_COMPILER_FLAGS()
  set(COVERAGE_LCOV_EXCLUDES 
    '${PROJECT_SOURCE_DIR}/examples/*' 
    '${PROJECT_SOURCE_DIR}/src/base64.*'   # external copy/pasted
    '${PROJECT_SOURCE_DIR}/src/mongoose.*' # external copy/pasted
    '${PROJECT_SOURCE_DIR}/src/sqlite3.*'  # external copy/pasted
    '${PROJECT_SOURCE_DIR}/src/mock/*'
    '${PROJECT_SOURCE_DIR}/external/*')

  setup_target_for_coverage_lcov(NAME test-coverage
    EXECUTABLE test-containers
    COMMAND test-linq-netw
    COMMAND test-device
    COMMAND test-devices
    COMMAND test-http
    COMMAND test-db #TODO remove from test coverage if WITHOUT SQLITE
    COMMAND test-jsmn
    COMMAND binding-linq-netw-cpp-test)

endif()

add_subdirectory(src)
add_subdirectory(examples/server)
add_subdirectory(bindings/cpp)
