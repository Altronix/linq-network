cmake_minimum_required(VERSION 3.11)
project(linq-netw)

option(BUILD_DEPENDENCIES "Whether or not to build the tests" ON)
set(LINQ_BUILD_DEPENDENCIES
    ${BUILD_DEPENDENCIES}
    CACHE BOOL "Build the external directory for linq-netw")

# dependency install
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
find_package(Threads)
set(deps_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/install)
file(MAKE_DIRECTORY ${deps_INSTALL_DIR}/include)
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
endif()

if(ENABLE_TESTING)
  set(CMAKE_BUILD_TYPE Debug)
  enable_testing()
  include(CTest)
  include(cmake/cmocka.cmake)
  include(cmake/CodeCoverage.cmake)
  include(cmake/test-fixture.cmake)
  append_coverage_compiler_flags()
  add_subdirectory(test/_helpers)
  add_subdirectory(test/_fixture)
  add_subdirectory(test/_mocks)
  add_subdirectory(test/containers)
  add_subdirectory(test/linq-netw)
  add_subdirectory(test/device)
  add_subdirectory(test/devices)
  add_subdirectory(test/integration)
  add_subdirectory(bindings/cpp/test)

  # add test coverage
  set(COVERAGE_LCOV_EXCLUDES test/**/* external/**/*)

  setup_target_for_coverage_lcov(NAME
                                 test-coverage
                                 EXECUTABLE
                                 test-containers
                                 COMMAND
                                 test-linq-netw
                                 COMMAND
                                 test-device
                                 COMMAND
                                 test-devices
                                 COMMAND
                                 integration-alert-test
                                 COMMAND
                                 integration-broadcast-test
                                 COMMAND
                                 integration-response-test
                                 COMMAND
                                 integration-forward-response-test
                                 COMMAND
                                 integration-504-test
                                 COMMAND
                                 binding-linq-netw-cpp-test)

endif()

add_subdirectory(src)
add_subdirectory(examples/server)
add_subdirectory(bindings/cpp)
