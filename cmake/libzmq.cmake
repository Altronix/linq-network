# libzmq

ExternalProject_Add(zmq-project
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/libzmq
	INSTALL_DIR ${deps_INSTALL_DIR}
	UPDATE_COMMAND ""
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=${deps_INSTALL_DIR} 
		-DZMQ_BUILD_TESTS:BOOL=OFF 
		-DBUILD_TESTS:BOOL=OFF 
		-DBUILD_STATIC:BOOL=ON
		-DCMAKE_BUILD_TYPE=Debug
	)

ExternalProject_Get_Property(zmq-project install_dir)
set(zmq_LIBRARY ${install_dir}/lib64/${CMAKE_STATIC_LIBRARY_PREFIX}zmq${CMAKE_STATIC_LIBRARY_SUFFIX})
set(zmq_INCLUDE_DIR ${install_dir}/include)

add_library(zmq STATIC IMPORTED)
set_property(TARGET zmq PROPERTY IMPORTED_LOCATION ${zmq_LIBRARY})
set_property(TARGET zmq PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${zmq_INCLUDE_DIR})
add_dependencies(zmq zmq-project)
