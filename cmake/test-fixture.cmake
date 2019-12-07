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
	PREFIX ${CMAKE_INSTALL_PREFIX}
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
add_definitions(-DCONFIG_LINUX_EMU)
add_definitions(-DUSYS_CONFIG_UNIX)
add_definitions(-DUZMTP_USE_TLS)
add_subdirectory(${linq_common_SOURCE_DIR}/libcrypt/wolfssl)
add_subdirectory(${linq_common_SOURCE_DIR}/libosal)
add_subdirectory(${linq_common_SOURCE_DIR}/libuzmtp)
add_subdirectory(${linq_common_SOURCE_DIR}/libitem)
add_subdirectory(${linq_common_SOURCE_DIR}/libatx)
add_subdirectory(${linq_common_SOURCE_DIR}/test/mocks)
target_include_directories(atx PUBLIC ${linq_common_SOURCE_DIR}/libatx)
