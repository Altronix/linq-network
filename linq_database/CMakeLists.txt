set(SOURCES database.c sqlite3.c)
set(HEADERS database.h sqlite3.h)
set(INSTALL_HEADERS database.h)
set(INCS ./)

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
else()
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
endif()

set(LIBS_STATIC ${LIBS} ${JSMN_WEB_TOKENS_STATIC_LIBS} linq-utils-static)
set(LIBS_SHARED ${LIBS} ${JSMN_WEB_TOKENS_SHARED_LIBS} linq-utils-shared)

# Build static library
add_library(linq-database-static STATIC ${SOURCES} ${HEADERS})
target_link_libraries(linq-database-static ${LIBS_STATIC})
target_include_directories(linq-database-static PUBLIC ${INCS})
set_target_properties(linq-database-static PROPERTIES
  PUBLIC_HEADER "${INSTALL_HEADERS}"
  OUTPUT_NAME "linq-database")
install(TARGETS linq-database-static
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/altronix)

# Build shared library
add_library(linq-database-shared SHARED ${SOURCES} ${HEADERS})
target_link_libraries(linq-database-shared ${LIBS_SHARED})
target_include_directories(linq-database-shared PRIVATE ${INCS})
set_target_properties(linq-database-shared PROPERTIES
  PUBLIC_HEADER "${INSTALL_HEADERS}"
  OUTPUT_NAME "linq-database")
install(TARGETS linq-database-shared
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include/altronix)
