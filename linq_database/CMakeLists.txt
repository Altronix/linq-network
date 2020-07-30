set(SOURCES database.c user.c alert.c device.c sqlite3.c)
set(HEADERS database.h user.h alert.h device.h sqlite3.h)
set(INSTALL_HEADERS database.h)
set(INCS ./)

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
else()
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
endif()

set(LIBS linq-utils-static)

append_log_level_compiler_flags(definitions)

# Build static library
if(BUILD_STATIC)
  add_library(linq-database-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-database-static ${LIBS})
  target_include_directories(linq-database-static PUBLIC ${INCS})
  target_compile_definitions(linq-database-static PUBLIC 
    ${definitions}
    "LINQ_DATABASE_STATIC")
  set_target_properties(linq-database-static PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-database")
  install(TARGETS linq-database-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

# Build shared library
if(BUILD_SHARED)
  add_library(linq-database-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(linq-database-shared ${LIBS})
  target_include_directories(linq-database-shared PUBLIC ${INCS})
  target_compile_definitions(linq-database-shared PUBLIC 
    "DLL_EXPORT" 
    ${definitions})
  set_target_properties(linq-database-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-database")
  install(TARGETS linq-database-shared
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
