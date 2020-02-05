# libjwt

ExternalProject_Add(jwt-project
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/libjwt
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
            -DBUILD_SHARED_LIBS:BOOL=OFF
            -DBUILD_EXAMPLES:BOOL=OFF
            -DENABLE_PIC:BOOL=ON
            -DJANSSON_LIBRARY:PATH=${jansson_static_LIBRARY}
            -DJANSSON_INCLUDE_DIR:PATH=${jansson_INCLUDE_DIR})

ExternalProject_Get_Property(jwt-project INSTALL_DIR)
set(jwt_INCLUDE_DIR ${INSTALL_DIR}/include)
FILE(MAKE_DIRECTORY ${INSTALL_DIR}/include)
add_dependencies(jwt-project jansson-project)

if(NOT MSVC)
  set(jwt_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}jwt${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(jwt_static_LIBRARY ${install_dir}/lib/${jwt_static_LIBRARY})
else()
  set(jwt_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}jwt${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(jwt_static_LIBRARY ${install_dir}/lib/${jwt_static_LIBRARY})
endif()

# jwt-static
add_library(jwt-static STATIC IMPORTED)
set_property(TARGET jwt-static PROPERTY IMPORTED_LOCATION ${jwt_static_LIBRARY})
set_property(TARGET jwt-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${jwt_INCLUDE_DIR})
add_dependencies(jwt-static jwt-project)
add_dependencies(jwt-static jansson-project)
