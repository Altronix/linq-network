# libzmq

set(ZMQ_VERSION "4.3.4")
set(ZMQ_SOURCE_DIR "${EXTERNAL_DIR}/zeromq-${ZMQ_VERSION}")
set(ZMQ_TEST_FILE "${ZMQ_SOURCE_DIR}/CMakeLists.txt")
check_extract("${DOWNLOAD_DIR}/zeromq-${ZMQ_VERSION}.tar.gz" "${ZMQ_TEST_FILE}")

ExternalProject_Add(zmq-project
	SOURCE_DIR ${ZMQ_SOURCE_DIR}
	INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
	UPDATE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND
		cmake
		--build .
		--target install
		--config MinSizeRel
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_INSTALL_LIBDIR=<INSTALL_DIR>/lib
		-DZMQ_BUILD_TESTS:BOOL=OFF 
		-DENABLE_WS:BOOL=OFF
		-DENABLE_CURVE:BOOL=OFF
		-DBUILD_TESTS:BOOL=OFF 
		-DBUILD_STATIC:BOOL=ON
		-DBUILD_SHARED:BOOL=ON
		-DWITH_DOCS:BOOL=OFF
		-DWITH_PERF_TOOL:BOOL=OFF
	)

ExternalProject_Get_Property(zmq-project install_dir)
set(zmq_INCLUDE_DIR ${install_dir}/include)
FILE(MAKE_DIRECTORY ${install_dir}/include)
IF(NOT MSVC)
  # Get the version of the ZMQ library
  execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/read_zmq_version.sh
    ${EXTERNAL_DIR}/libzmq/include/zmq.h
    OUTPUT_VARIABLE zmq_VERSION)
  set(zmq_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}zmq${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(zmq_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}zmq${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(zmq_static_LIBRARY_LOC ${install_dir}/lib/${zmq_static_LIBRARY})
  set(zmq_shared_LIBRARY_LOC ${install_dir}/lib/${zmq_shared_LIBRARY})
ELSE()
  execute_process(COMMAND powershell -ExecutionPolicy Bypass
    -File "${CMAKE_SOURCE_DIR}/scripts/read_zmq_version.ps1"
    "${ZMQ_SOURCE_DIR}/include/zmq.h"
    OUTPUT_VARIABLE zmq_VERSION)
  STRING(REGEX REPLACE "\n" "" zmq_VERSION ${zmq_VERSION})
  STRING(REGEX REPLACE "\r" "" zmq_VERSION ${zmq_VERSION})
  MESSAGE(STATUS "zmq_VERSION: ${zmq_VERSION}")
  set(zmq_static_LIBRARY libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-s-${zmq_VERSION}${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(zmq_shared_LIBRARY libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-${zmq_VERSION}${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(zmq_static_LIBRARY_LOC ${install_dir}/lib/${zmq_static_LIBRARY})
  set(zmq_shared_LIBRARY_LOC ${install_dir}/bin/${zmq_shared_LIBRARY})
ENDIF()

# zmq-static
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/libzmq-static-loc.txt ${zmq_static_LIBRARY})
add_library(zmq-static STATIC IMPORTED)
set_property(TARGET zmq-static PROPERTY IMPORTED_LOCATION ${zmq_static_LIBRARY_LOC})
set_property(TARGET zmq-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${zmq_INCLUDE_DIR})
add_dependencies(zmq-static zmq-project)

# zmq-shared
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/libzmq-shared-loc.txt ${zmq_shared_LIBRARY})
add_library(zmq-shared STATIC IMPORTED)
set_property(TARGET zmq-shared PROPERTY IMPORTED_LOCATION ${zmq_shared_LIBRARY_LOC})
set_property(TARGET zmq-shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${zmq_INCLUDE_DIR})
add_dependencies(zmq-shared zmq-project)
