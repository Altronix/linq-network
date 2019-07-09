cmake_minimum_required(VERSION 3.11)
project(linq-c)

# add dependencies
include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)
include(cmake/libzmq.cmake)
include(cmake/czmq.cmake)

set(dependency-libraries libzmq czmq)

add_subdirectory(src)

IF(ENABLE_TESTING)
    	enable_testing()
    	include(CTest)
	include(cmake/cmocka.cmake)
    	include(cmake/cmocka.cmake)
    	include(cmake/CodeCoverage.cmake)
	# add_subdirectory(test/...
	# add_subdirectory(test/...
	# add_subdirectory(test/...
ENDIF()
