set(JSMN_WEB_TOKENS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/jsmn-web-tokens)
set(JSMN_WEB_TOKENS_TEST_FILE ${JSMN_WEB_TOKENS_SOURCE_DIR}/CMakeLists.txt)
if(USE_SYSTEM_JSMN_WEB_TOKENS)
    find_package(jsmn-web-tokens REQUIRED)
    set(JSMN_WEB_TOKENS_STATIC_LIBRARIES jsmn-web-tokens)
    set(JSMN_WEB_TOKENS_SHARED_LIBRARIES jsmn-web-tokens)
else()
    if(GIT_FOUND AND NOT EXISTS "${JSMN_WEB_TOKENS_TEST_FILE}")
        message(STATUS "Downloading jsmn-web-tokens submodule")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init jsmn-web-tokens
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE JSMN_WEB_TOKENS_GIT_CLONE_RESULT)
        message(STATUS "jsmn-web-tokens download result: ${JSMN_WEB_TOKENS_GIT_CLONE_RESULT}")
    endif()
    set(JSMN_WEB_TOKENS_STATIC_LIBRARIES jsmn-web-tokens-static)
    set(JSMN_WEB_TOKENS_SHARED_LIBRARIES jsmn-web-tokens-shared)
endif()

if(NOT MSVC)
    ExternalProject_Add(jsmn-web-tokens-project
        SOURCE_DIR ${JSMN_WEB_TOKENS_SOURCE_DIR}
	INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
        UPDATE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND
            cmake
            --build .
            --target install
            --config Release
        LIST_SEPARATOR |
        CMAKE_ARGS
	    -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DWITH_CRYPTO:STRING=OPENSSL
            -DWITH_SYSTEM_DEPENDENCIES:BOOL=ON)
    ExternalProject_Get_Property(jsmn-web-tokens-project INSTALL_DIR)
    set(JSMN_WEB_TOKENS_INCLUDE_DIR ${INSTALL_DIR}/include)
    FILE(MAKE_DIRECTORY ${INSTALL_DIR}/include)

    set(JSMN_WEB_TOKENS_STATIC_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}jsmn-web-tokens${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(JSMN_WEB_TOKENS_SHARED_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}jsmn-web-tokens${CMAKE_SHARED_LIBRARY_SUFFIX})
    set(JSMN_WEB_TOKENS_STATIC_LIBRARY ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}jsmn-web-tokens${CMAKE_STATIC_LIBRARY_SUFFIX})
    set(JSMN_WEB_TOKENS_SHARED_LIBRARY ${INSTALL_DIR}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}jsmn-web-tokens${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
endif()

# jsmn-web-tokens-static
add_library(jsmn-web-tokens-static STATIC IMPORTED)
set_property(TARGET jsmn-web-tokens-static PROPERTY IMPORTED_LOCATION ${JSMN_WEB_TOKENS_STATIC_LIBRARY})
set_property(TARGET jsmn-web-tokens-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${JSMN_WEB_TOKENS_INCLUDE_DIR})
add_dependencies(jsmn-web-tokens-static jsmn-web-tokens-project)

# jsmn-web-tokens-shared
add_library(jsmn-web-tokens-shared STATIC IMPORTED)
set_property(TARGET jsmn-web-tokens-shared PROPERTY IMPORTED_LOCATION ${JSMN_WEB_TOKENS_SHARED_LIBRARY})
set_property(TARGET jsmn-web-tokens-shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${JSMN_WEB_TOKENS_INCLUDE_DIR})
add_dependencies(jsmn-web-tokens-shared jsmn-web-tokens-project)
