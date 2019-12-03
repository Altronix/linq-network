# libzmq

ExternalProject_Add(zmq-project
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/libzmq
	INSTALL_DIR ${deps_INSTALL_DIR}
	UPDATE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND
		cmake
		--build .
		--target install
		--config Release
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=${deps_INSTALL_DIR} 
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
IF(NOT MSVC)
  set(zmq_LIBRARY ${install_dir}/lib64/${CMAKE_STATIC_LIBRARY_PREFIX}zmq${CMAKE_STATIC_LIBRARY_SUFFIX})
ELSE()
  set(zmq_LIBRARY ${install_dir}/lib/libzmq-v142-mt-s-4_3_3${CMAKE_STATIC_LIBRARY_SUFFIX})
ENDIF()

add_library(zmq STATIC IMPORTED)
set_property(TARGET zmq PROPERTY IMPORTED_LOCATION ${zmq_LIBRARY})
set_property(TARGET zmq PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${zmq_INCLUDE_DIR})
add_dependencies(zmq zmq-project)
