# libzmq

ExternalProject_Add(zmq-project
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/libzmq
	INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
	UPDATE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND
		cmake
		--build .
		--target install
		--config Release
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_INSTALL_LIBDIR=<INSTALL_DIR>/lib
		-DZMQ_BUILD_TESTS:BOOL=OFF 
		-DDISABLE_WS:BOOL=ON
		-DENABLE_CURVE:BOOL=OFF
		-DBUILD_TESTS:BOOL=OFF 
		-DBUILD_STATIC:BOOL=ON
		-DBUILD_SHARED:BOOL=ON
		-DWITH_PERF_TOOL:BOOL=OFF
	)

ExternalProject_Get_Property(zmq-project install_dir)
set(zmq_INCLUDE_DIR ${install_dir}/include)
FILE(MAKE_DIRECTORY ${install_dir}/include)
IF(NOT MSVC)
  # Get the version of the ZMQ library
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/read_zmq_version.sh
    ${CMAKE_SOURCE_DIR}/external/libzmq/include/zmq.h
    OUTPUT_VARIABLE zmq_VERSION)
  set(zmq_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}zmq${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(zmq_LIBRARY_LOC ${install_dir}/lib/${zmq_LIBRARY})
ELSE()
  execute_process(COMMAND powershell -ExecutionPolicy Bypass
    -File "${CMAKE_SOURCE_DIR}/scripts/read_zmq_version.ps1"
          "${CMAKE_SOURCE_DIR}/external/libzmq/include/zmq.h"
    OUTPUT_VARIABLE zmq_VERSION)
  STRING(REGEX REPLACE "\n" "" zmq_VERSION ${zmq_VERSION})
  STRING(REGEX REPLACE "\r" "" zmq_VERSION ${zmq_VERSION})
  MESSAGE(STATUS "zmq_VERSION: ${zmq_VERSION}")
  set(zmq_LIBRARY libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-s-${zmq_VERSION}${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(zmq_LIBRARY_LOC ${install_dir}/lib/${zmq_LIBRARY})
ENDIF()

FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/libzmq-loc.txt ${zmq_LIBRARY})
add_library(zmq STATIC IMPORTED)
set_property(TARGET zmq PROPERTY IMPORTED_LOCATION ${zmq_LIBRARY_LOC})
set_property(TARGET zmq PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${zmq_INCLUDE_DIR})
add_dependencies(zmq zmq-project)
