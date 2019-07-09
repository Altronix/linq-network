# czmq

ExternalProject_Add(czmq-project
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/czmq
	UPDATE_COMMAND ""
	INSTALL_COMMAND ""
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
	)

ExternalProject_Get_Property(czmq-project install_dir)

add_library(czmq STATIC IMPORTED)
add_dependencies(czmq-project zmq-project)
add_dependencies(czmq czmq-project)

set_property(TARGET czmq 
	PROPERTY IMPORTED_LOCATION 
	${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}czmq${CMAKE_STATIC_LIBRARY_SUFFIX}
	)
set_property(TARGET czmq 
	PROPERTY INTERFACE_INCLUDE_DIRECTORIES 
	${install_dir}/include
	)
