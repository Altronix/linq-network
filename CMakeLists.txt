cmake_minimum_required(VERSION 3.11)
project(linq-c)


# dependency install
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
find_package(Threads)
set(deps_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/install)
file(MAKE_DIRECTORY ${deps_INSTALL_DIR}/include)
set(deps_LIBRARIES czmq zmq ${CMAKE_THREAD_LIBS_INIT} rt m stdc++)
set(deps_TEST_LIBRARIES cmocka mock-czmq czmq zmq ${CMAKE_THREAD_LIBS_INIT} rt m stdc++)
include(cmake/libzmq.cmake)
include(cmake/czmq.cmake)

add_subdirectory(src)

IF(ENABLE_TESTING)
    	enable_testing()
    	include(CTest)
	include(cmake/cmocka.cmake)
	# include(cmake/CodeCoverage.cmake)
	add_subdirectory(test/mocks)
	add_subdirectory(test/linq)
ENDIF()
