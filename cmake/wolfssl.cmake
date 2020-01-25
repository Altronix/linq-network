set(wolfssl_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/wolfssl)

### Build Wolfssl ###
ExternalProject_Add(wolfssl-project
	SOURCE_DIR ${wolfssl_SOURCE_DIR}
	INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
	BUILD_IN_SOURCE 1
	UPDATE_COMMAND ""
	PREFIX ${CMAKE_INSTALL_PREFIX}
	CONFIGURE_COMMAND ./autogen.sh
		COMMAND ./configure --prefix=<INSTALL_DIR>
		--enable-debug
		--enable-static=yes
		--enable-shared=yes
		--enable-oldtls=no
		--enable-keygen
		--enable-certgen
		--enable-testcert
                --enable-opensslextra
	BUILD_COMMAND make
	INSTALL_COMMAND make install
	EXCLUDE_FROM_ALL true)
ExternalProject_Get_Property(wolfssl-project INSTALL_DIR)
set(wolfssl_INCLUDE_DIR ${INSTALL_DIR}/include)
FILE(MAKE_DIRECTORY ${INSTALL_DIR}/include)
if(NOT MSVC)
  set(wolfssl_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}wolfssl${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(wolfssl_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}wolfssl${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(wolfssl_static_LIBRARY ${INSTALL_DIR}/lib/${wolfssl_static_LIBRARY})
  set(wolfssl_shared_LIBRARY ${INSTALL_DIR}/lib/${wolfssl_shared_LIBRARY})
else()
  set(wolfssl_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}wolfssl${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(wolfssl_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}wolfssl${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(wolfssl_static_LIBRARY ${INSTALL_DIR}/lib/${wolfssl_static_LIBRARY})
  set(wolfssl_shared_LIBRARY ${INSTALL_DIR}/lib/${wolfssl_shared_LIBRARY})
endif()

# wolfssl-static
add_library(wolfssl-static STATIC IMPORTED)
add_dependencies(wolfssl-static wolfssl-project)
set_property(TARGET wolfssl-static PROPERTY IMPORTED_LOCATION ${wolfssl_static_LIBRARY})
set_property(TARGET wolfssl-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${wolfssl_INCLUDE_DIR})

# alias for linq-common
add_library(wolfssl STATIC IMPORTED)
add_dependencies(wolfssl wolfssl-project)
set_property(TARGET wolfssl PROPERTY IMPORTED_LOCATION ${wolfssl_static_LIBRARY})
set_property(TARGET wolfssl PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${wolfssl_INCLUDE_DIR})

# wolfssl-shared
add_library(wolfssl-shared STATIC IMPORTED)
add_dependencies(wolfssl-shared wolfssl-project)
set_property(TARGET wolfssl-shared PROPERTY IMPORTED_LOCATION ${wolfssl_shared_LIBRARY})
set_property(TARGET wolfssl-shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${wolfssl_INCLUDE_DIR})
