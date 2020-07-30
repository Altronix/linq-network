# Main source files
set(SOURCES base64.c json.c)
set(HEADERS base64.h klib/khash.h klib/klist.h containers.h jsmn.h json.h log.h)

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
  list(APPEND INCS ./sys/unix)
  list(APPEND SOURCES sys/unix/sys.c)
  list(APPEND HEADERS sys/unix/sys.h)
else()
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
  list(APPEND INCS ./sys/win)
  list(APPEND SOURCES sys/win/sys.c)
  list(APPEND HEADERS sys/win/sys.h)
endif()

list(APPEND INCS ./)

# Build a static library
if(BUILD_STATIC)
  add_library(linq-utils-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-utils-static ${LIBS})
  target_compile_definitions(linq-utils-static PUBLIC "LINQ_UTILS_STATIC")
  target_include_directories(linq-utils-static PUBLIC ${INCS})
  set_target_properties(linq-utils-static PROPERTIES
    PUBLIC_HEADER "${HEADERS}"
    OUTPUT_NAME "linq-utils")
  install(TARGETS linq-utils-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

# Build a shared library
if(BUILD_SHARED)
  add_library(linq-utils-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(linq-utils-shared ${LIBS})
  target_compile_definitions(linq-utils-shared PUBLIC "DLL_EXPORT")
  target_include_directories(linq-utils-shared PUBLIC ${INCS})
  set_target_properties(linq-utils-shared PROPERTIES
    PUBLIC_HEADER "${HEADERS}"
    OUTPUT_NAME "linq-utils")
  install(TARGETS linq-utils-shared
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
