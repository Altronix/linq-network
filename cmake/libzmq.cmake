# libzmq

ExternalProject_Add(zmq-project
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/libzmq
	UPDATE_COMMAND ""
	INSTALL_COMMAND ""
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX} 
		-DZMQ_BUILD_TESTS:BOOL=OFF 
		-DBUILD_TESTS:BOOL=OFF 
		-DBUILD_STATIC:BOOL=ON
		-DCMAKE_BUILD_TYPE=Debug
	)

ExternalProject_Get_Property(zmq-project install_dir)

add_library(zmq SHARED IMPORTED)
set_property(TARGET zmq 
	PROPERTY IMPORTED_LOCATION 
	${install_dir}/lib64/${CMAKE_SHARED_LIBRARY_PREFIX}zmq${CMAKE_SHARED_LIBRARY_SUFFIX}
	)
set_property(TARGET zmq 
	PROPERTY INTERFACE_INCLUDE_DIRECTORIES 
	${install_dir}/include
	)
add_dependencies(zmq zmq-project)
