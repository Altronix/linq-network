cmake_minimum_required(VERSION 3.11)
project(linq-netw)

# Compiler stuff
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_DEPENDENCIES "Whether or not to build the tests" ON)
option(WITH_MONGOOSE "Include HTTP support" ON)
option(WITH_SQLITE "Include database support" ON)

set(LINQ_BUILD_DEPENDENCIES
    ${BUILD_DEPENDENCIES}
    CACHE BOOL "Build the external directory for linq-netw")
set(LINQ_LOG_LEVEL
  "TRACE"
  CACHE STRING 
  "set logging to TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NONE")

# dependency install
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
find_package(Threads)
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
    COMMAND binding-linq-netw-cpp-test)
endif()

add_subdirectory(src)
add_subdirectory(examples/server)
add_subdirectory(bindings/cpp)
