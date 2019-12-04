# czmq

ExternalProject_Add(czmq-project
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/czmq
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
		-DCZMQ_WITH_SYSTEMD:BOOL=OFF
		-DENABLE_DRAFTS:BOOL=ON
	)

ExternalProject_Get_Property(czmq-project install_dir)
set(czmq_INCLUDE_DIR ${install_dir}/include)
IF(NOT MSVC)
  set(czmq_LIBRARY ${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}czmq${CMAKE_STATIC_LIBRARY_SUFFIX})
ELSE()
  set(czmq_LIBRARY ${install_dir}/lib/libczmq${CMAKE_STATIC_LIBRARY_SUFFIX})
ENDIF()

add_library(czmq STATIC IMPORTED)
add_dependencies(czmq-project zmq-project)
add_dependencies(czmq czmq-project)

set_property(TARGET czmq PROPERTY IMPORTED_LOCATION ${czmq_LIBRARY})
set_property(TARGET czmq PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${czmq_INCLUDE_DIR})
