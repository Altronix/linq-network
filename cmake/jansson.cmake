# libjansson

ExternalProject_Add(jansson-project
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/jansson
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
            -DJANSSON_BUILD_DOCS:BOOL=OFF
            -DJANSSON_BUILD_SHARED_LIBS:BOOL=OFF
            -DJANSSON_EXAMPLES:BOOL=OFF
            -DJANSSON_WITHOUT_TESTS:BOOL=ON)

ExternalProject_Get_Property(jansson-project INSTALL_DIR)
set(jansson_INCLUDE_DIR ${INSTALL_DIR}/include)
FILE(MAKE_DIRECTORY ${INSTALL_DIR}/include)

if(NOT MSVC)
  set(jansson_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}jansson${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(jansson_static_LIBRARY ${INSTALL_DIR}/lib/${jansson_static_LIBRARY})
else()
  set(jansson_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}jansson${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(jansson_static_LIBRARY ${INSTALL_DIR}/lib/${jansson_static_LIBRARY})
endif()

# jansson-static
add_library(jansson-static STATIC IMPORTED)
set_property(TARGET jansson-static PROPERTY IMPORTED_LOCATION ${jansson_static_LIBRARY})
set_property(TARGET jansson-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${jansson_INCLUDE_DIR})
add_dependencies(jansson-static jansson-project)
