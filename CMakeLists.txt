cmake_minimum_required(VERSION 3.11)
project(linq-c)

# add dependencies
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
include(cmake/libzmq.cmake)
include(cmake/czmq.cmake)

set(deps-libs libzmq czmq)
set(test-libs cmocka mock-czmq)

add_subdirectory(src)

IF(ENABLE_TESTING)
    	enable_testing()
    	include(CTest)
	include(cmake/cmocka.cmake)
	# include(cmake/CodeCoverage.cmake)
	add_subdirectory(test/mocks)
	add_subdirectory(test/linq)
ENDIF()
