# czmq

set(CZMQ_SOURCE_DIR ${EXTERNAL_DIR}/czmq)

if(NOT EXISTS ${CZMQ_SOURCE_DIR}/CMakeLists.txt)
	execute_process(
	    COMMAND ${GIT_EXECUTABLE} submodule update --init ${EXTERNAL_DIR}/czmq
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		RESULT_VARIABLE GIT_CLONE_CZMQ_RESULT)
	message(STATUS "GIT_CLONE_CZMQ_RESULT: ${GIT_CLONE_CZMQ_RESULT}")
endif()

ExternalProject_Add(czmq-project
	SOURCE_DIR ${CZMQ_SOURCE_DIR}
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
		-DCZMQ_WITH_SYSTEMD:BOOL=OFF
		-DENABLE_DRAFTS:BOOL=ON
	)

ExternalProject_Get_Property(czmq-project install_dir)
set(czmq_INCLUDE_DIR ${install_dir}/include)
FILE(MAKE_DIRECTORY ${install_dir}/include)
IF(NOT MSVC)
  set(czmq_static_LIBRARY ${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}czmq${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(czmq_shared_LIBRARY ${install_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}czmq${CMAKE_SHARED_LIBRARY_SUFFIX})
ELSE()
  set(czmq_static_LIBRARY ${install_dir}/lib/libczmq${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(czmq_shared_LIBRARY ${install_dir}/bin/libczmq${CMAKE_SHARED_LIBRARY_SUFFIX})
  message(STATUS "czmq_shared_LIBRARY: ${czmq_shared_LIBRARY}")
ENDIF()

# czmq-static
add_library(czmq-static STATIC IMPORTED)
add_dependencies(czmq-project zmq-project)
add_dependencies(czmq-static czmq-project)
set_property(TARGET czmq-static PROPERTY IMPORTED_LOCATION ${czmq_static_LIBRARY})
set_property(TARGET czmq-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${czmq_INCLUDE_DIR})

# czmq-shared
add_library(czmq-shared SHARED IMPORTED)
add_dependencies(czmq-project zmq-project)
add_dependencies(czmq-shared czmq-project)
set_property(TARGET czmq-shared PROPERTY IMPORTED_LOCATION ${czmq_shared_LIBRARY})
set_property(TARGET czmq-shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${czmq_INCLUDE_DIR})
