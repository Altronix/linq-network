# cmocka

set(CMOCKA_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/cmocka-1.1.5)
if(NOT EXISTS ${CMOCKA_SOURCE_DIR}/CMakeLists.txt)
	execute_process(
		COMMAND tar -xzvf ${CMAKE_SOURCE_DIR}/dl/cmocka-1.1.5.tar.gz 
			    -C ${CMAKE_SOURCE_DIR}/external
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		RESULT_VARIABLE EXTRACT_CMOCKA_RESULT)
	message(STATUS "EXTRACT_CMOCKA_RESULT: ${EXTRACT_CMOCKA_RESULT}")
endif()

ExternalProject_Add(cmocka-project
	SOURCE_DIR ${CMOCKA_SOURCE_DIR}
	INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
	UPDATE_COMMAND ""
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		-DCMAKE_INSTALL_LIBDIR=<INSTALL_DIR>/lib
		-DWITH_STATIC_LIB:BOOL=ON 
		-DUNIT_TESTING:BOOL=OFF
	)

ExternalProject_Get_Property(cmocka-project install_dir)
set(cmocka_LIBRARY ${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cmocka-static${CMAKE_STATIC_LIBRARY_SUFFIX})
set(cmocka_INCLUDE_DIR ${install_dir}/include)

add_library(cmocka STATIC IMPORTED)
set_property(TARGET cmocka PROPERTY IMPORTED_LOCATION ${cmocka_LIBRARY})
set_property(TARGET cmocka PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${cmocka_INCLUDE_DIR})
add_dependencies(cmocka cmocka-project)
