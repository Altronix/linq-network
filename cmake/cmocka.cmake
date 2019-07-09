# cmocka

set(cmocka_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/cmocka)

ExternalProject_Add(cmocka-project
	SOURCE_DIR ${cmocka_SOURCE_DIR}
	UPDATE_COMMAND ""
	INSTALL_COMMAND ""
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
		-DWITH_STATIC_LIB:BOOL=ON 
		-DUNIT_TESTING:BOOL=OFF
	)

ExternalProject_Get_Property(cmocka-project install_dir)
set(cmocka_LIBRARY ${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}cmocka${CMAKE_STATIC_LIBRARY_SUFFIX})
set(cmocka_INCLUDE_DIR ${cmocka_SOURCE_DIR}/include)

add_library(cmocka STATIC IMPORTED)
set_property(TARGET cmocka PROPERTY IMPORTED_LOCATION ${cmocka_LIBRARY})
set_property(TARGET cmocka PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${cmocka_SOURCE_DIR}/include)
add_dependencies(cmocka cmocka-project)
