# libjwt

ExternalProject_Add(jwt-project-shared
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
            -DBUILD_SHARED_LIBS:BOOL=ON
            -DBUILD_EXAMPLES:BOOL=OFF
            -DENABLE_PIC:BOOL=ON
            -DJANSSON_LIBRARY:PATH=${jansson_static_LIBRARY}
            -DJANSSON_INCLUDE_DIR:PATH=${jansson_INCLUDE_DIR})

ExternalProject_Add(jwt-project-static
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

ExternalProject_Get_Property(jwt-project-shared INSTALL_DIR)
set(jwt_INCLUDE_DIR ${INSTALL_DIR}/include)
FILE(MAKE_DIRECTORY ${INSTALL_DIR}/include)
add_dependencies(jwt-project-shared jansson-project)
add_dependencies(jwt-project-static jansson-project)

if(NOT MSVC)
  set(jwt_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}jwt${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(jwt_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}jwt${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(jwt_static_LIBRARY ${install_dir}/lib/${jwt_static_LIBRARY})
  set(jwt_shared_LIBRARY ${install_dir}/lib/${jwt_shared_LIBRARY})
else()
  set(jwt_static_LIBRARY ${CMAKE_STATIC_LIBRARY_PREFIX}jwt${CMAKE_STATIC_LIBRARY_SUFFIX})
  set(jwt_shared_LIBRARY ${CMAKE_SHARED_LIBRARY_PREFIX}jwt${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(jwt_static_LIBRARY ${install_dir}/lib/${jwt_static_LIBRARY})
  set(jwt_shared_LIBRARY ${install_dir}/lib/${jwt_shared_LIBRARY})
endif()

# jwt-static
add_library(jwt-static STATIC IMPORTED)
set_property(TARGET jwt-static PROPERTY IMPORTED_LOCATION ${jwt_static_LIBRARY})
set_property(TARGET jwt-static PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${jwt_INCLUDE_DIR})
add_dependencies(jwt-static jwt-project-static)
add_dependencies(jwt-static jansson-project)

# jwt-shared
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/libjwt-shared-loc.txt ${jwt_shared_LIBRARY})
add_library(jwt-shared STATIC IMPORTED)
set_property(TARGET jwt-shared PROPERTY IMPORTED_LOCATION ${jwt_shared_LIBRARY})
set_property(TARGET jwt-shared PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${jwt_INCLUDE_DIR})
add_dependencies(jwt-shared jwt-project-shared)
add_dependencies(jwt-shared jansson-project)
