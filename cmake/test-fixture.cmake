# test-fixture

set(linq_common_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/linq-common)
set(http_parser_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/http_parser)
set(wolfssl_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/wolfssl)

### Build http_parser ###
add_library(http-parser 
	${http_parser_SOURCE_DIR}/http_parser.c 
	${http_parser_SOURCE_DIR}/http_parser.h)
target_include_directories(http-parser PUBLIC ${http_parser_SOURCE_DIR})

### Build Wolfssl ###
ExternalProject_Add(wolfssl_PROJECT
	SOURCE_DIR ${wolfssl_SOURCE_DIR}
	BUILD_IN_SOURCE 1
	UPDATE_COMMAND ""
	PREFIX ${deps_INSTALL_DIR}
	CONFIGURE_COMMAND ./autogen.sh
		COMMAND ./configure --prefix=<INSTALL_DIR>
		--enable-debug
		--enable-static
		--enable-shared=no
		--enable-oldtls=no
		--enable-keygen
		--enable-certgen
		--enable-testcert
	BUILD_COMMAND make
	INSTALL_COMMAND make install
	EXCLUDE_FROM_ALL true)
ExternalProject_Get_Property(wolfssl_PROJECT install_dir)
set(wolfssl_LIBRARY ${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}wolfssl${CMAKE_STATIC_LIBRARY_SUFFIX})
set(wolfssl_INCLUDE_DIR ${install_dir}/include)
add_library(wolfssl STATIC IMPORTED)
add_dependencies(wolfssl wolfssl_PROJECT)
set_property(TARGET wolfssl PROPERTY IMPORTED_LOCATION ${wolfssl_LIBRARY})
set_property(TARGET wolfssl PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${wolfssl_INCLUDE_DIR})

### build linq-common ###
ExternalProject_Add(linq_common_PROJECT
	SOURCE_DIR ${linq_common_SOURCE_DIR}
	PREFIX ${deps_INSTALL_DIR})
ExternalProject_Get_Property(linq_common_PROJECT install_dir)

ExternalProject_Add(atx-project
	SOURCE_DIR ${linq_common_SOURCE_DIR}
	UPDATE_COMMAND ""
	LIST_SEPARATOR |
	CMAKE_ARGS 
		-DCMAKE_INSTALL_PREFIX=${deps_INSTALL_DIR} 
		-DZMQ_BUILD_TESTS:BOOL=OFF 
		-DBUILD_TESTS:BOOL=OFF 
		-DBUILD_STATIC:BOOL=ON
		-DBUILD_WOLFSSL:BOOL=OFF
		-DBUILD_HTTP_PARSER:BOOL=OFF
		-DCMAKE_BUILD_TYPE=Debug
	)

add_library(atx STATIC IMPORTED)
add_library(osal STATIC IMPORTED)
add_library(crypt STATIC IMPORTED)
add_library(item STATIC IMPORTED)
add_library(uzmtp STATIC IMPORTED)

set_property(TARGET atx PROPERTY IMPORTED_LOCATION ${deps_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}atx${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET osal PROPERTY IMPORTED_LOCATION ${deps_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}osal${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET crypt PROPERTY IMPORTED_LOCATION ${deps_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}crypt${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET item PROPERTY IMPORTED_LOCATION ${deps_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}item${CMAKE_STATIC_LIBRARY_SUFFIX})
set_property(TARGET uzmtp PROPERTY IMPORTED_LOCATION ${deps_INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}uzmtp${CMAKE_STATIC_LIBRARY_SUFFIX})

set_property(TARGET atx PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${deps_INSTALL_DIR}/include)
set_property(TARGET osal PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${deps_INSTALL_DIR}/include)
set_property(TARGET crypt PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${deps_INSTALL_DIR}/include)
set_property(TARGET item PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${deps_INSTALL_DIR}/include)
set_property(TARGET uzmtp PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${deps_INSTALL_DIR}/include)

add_dependencies(atx atx-project)
add_dependencies(osal atx-project)
add_dependencies(crypt atx-project)
add_dependencies(item atx-project)
add_dependencies(uzmtp atx-project)
