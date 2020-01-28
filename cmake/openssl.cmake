if(NOT MSVC)
    ExternalProject_Add(openssl-project
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/openssl
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ./config no-tests --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR>/ssl
        BUILD_COMMAND make
        INSTALL_COMMAND make install_sw
        BUILD_IN_SOURCE ON
    )
    ExternalProject_Get_Property(openssl-project INSTALL_DIR)

    set(ssl_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}ssl${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(ssl_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}ssl${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(crypto_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}crypto${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(crypto_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}crypto${CMAKE_SHARED_LIBRARY_SUFFIX})

    set(ssl_static_LIBRARY ${INSTALL_DIR}/lib/${ssl_static_LIBRARY})
    set(ssl_shared_LIBRARY ${INSTALL_DIR}/lib/${ssl_shared_LIBRARY})
    set(crypto_static_LIBRARY ${INSTALL_DIR}/lib/${crypto_static_LIBRARY})
    set(crypto_shared_LIBRARY ${INSTALL_DIR}/lib/${crypto_shared_LIBRARY})
else()
    ExternalProject_Add(openssl-project
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/openssl
        INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ${PERL_EXECUTABLE} ./Configure VC-WIN64A no-asm no-tests --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR>/ssl
        BUILD_COMMAND nmake
        INSTALL_COMMAND nmake install_sw
        BUILD_IN_SOURCE ON
    )
    message(STATUS "PERL_EXECUTABLE: ${PERL_EXECUTABLE}")
    ExternalProject_Get_Property(openssl-project INSTALL_DIR)

    set(ssl_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}ssl${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(ssl_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}ssl${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(crypto_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}crypto${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(crypto_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}crypto${CMAKE_SHARED_LIBRARY_SUFFIX})

    set(ssl_static_LIBRARY ${INSTALL_DIR}/lib/${ssl_static_LIBRARY})
    set(ssl_shared_LIBRARY ${INSTALL_DIR}/lib/${ssl_shared_LIBRARY})
    set(crypto_static_LIBRARY ${INSTALL_DIR}/lib/${crypto_static_LIBRARY})
    set(crypto_shared_LIBRARY ${INSTALL_DIR}/lib/${crypto_shared_LIBRARY})
endif()

set(openssl_INCLUDE_DIR ${INSTALL_DIR}/include)
FILE(MAKE_DIRECTORY ${INSTALL_DIR}/include)

# ssl-static
add_library(ssl-static STATIC IMPORTED)
set_property(TARGET ssl-static PROPERTY IMPORTED_LOCATION ${ssl_static_LIBRARY})
set_property(TARGET ssl-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${openssl_INCLUDE_DIR})
add_dependencies(ssl-static openssl-project)

# ssl-shared
add_library(ssl-shared STATIC IMPORTED)
set_property(TARGET ssl-shared PROPERTY IMPORTED_LOCATION ${ssl_shared_LIBRARY})
set_property(TARGET ssl-shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${openssl_INCLUDE_DIR})
add_dependencies(ssl-shared openssl-project)

# crypto-static
add_library(crypto-static STATIC IMPORTED)
set_property(TARGET crypto-static PROPERTY IMPORTED_LOCATION ${crypto_static_LIBRARY})
set_property(TARGET crypto-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${openssl_INCLUDE_DIR})
add_dependencies(crypto-static opencrypto-project)

# crypto-shared
add_library(crypto-shared STATIC IMPORTED)
set_property(TARGET crypto-shared PROPERTY IMPORTED_LOCATION ${crypto_shared_LIBRARY})
set_property(TARGET crypto-shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${openssl_INCLUDE_DIR})
add_dependencies(crypto-shared openssl-project)
