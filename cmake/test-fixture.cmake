# test-fixture

set(linq_common_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/linq-common)
set(http_parser_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/http_parser)
set(wolfssl_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/wolfssl)

### Build http_parser ###
add_library(http-parser 
	${http_parser_SOURCE_DIR}/http_parser.c 
	${http_parser_SOURCE_DIR}/http_parser.h)
target_include_directories(http-parser PUBLIC ${http_parser_SOURCE_DIR})
# ExternalProject_Add(http_parser_PROJECT
# 	SOURCE_DIR ${http_parser_SOURCE_DIR}
# 	INSTALL_DIR ${deps_INSTALL_DIR}
# 	CONFIGURE_COMMAND ""
# 	BUILD_COMMAND make package PREFIX=${deps_INSTALL_DIR}
# 	INSTALL_COMMAND make install)
# ExternalProject_Get_Property(http_parser_PROJECT install_dir)
# set(http_parser_LIBRARY ${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}http-parser${CMAKE_STATIC_LIBRARY_SUFFIX})
# set(http_parser_INCLUDE_DIR ${install_dir}/include)
# add_library(http_parser STATIC IMPORTED)
# add_dependencies(http_parser http_parser_PROJECT)
# set_property(TARGET http_parser PROPERTY IMPORTED_LOCATION ${http_parser_LIBRARY})
# set_property(TARGET http_parser PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${http_parser_INCLUDE_DIR})

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

### TODO ### build linq-common
