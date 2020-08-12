# Main source files
set(SOURCES base64.c rlp.c json.c)
set(HEADERS 
  base64.h 
  json.h 
  jsmn.h 
  log.h
  containers.h
  klib/khash.h 
  klib/klist.h)

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
  add_library(utils-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(utils-static ${LIBS})
  target_compile_definitions(utils-static PUBLIC "LINQ_UTILS_STATIC")
  target_compile_definitions(utils-static PUBLIC "RLP_STATIC")
  target_include_directories(utils-static PUBLIC ${INCS})
  set_target_properties(utils-static PROPERTIES
    PUBLIC_HEADER "${HEADERS}"
    OUTPUT_NAME "linqutils")
  install(TARGETS utils-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

# Build a shared library
if(BUILD_SHARED)
  add_library(utils-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(utils-shared ${LIBS})
  target_compile_definitions(utils-shared PUBLIC "DLL_EXPORT")
  target_include_directories(utils-shared PUBLIC ${INCS})
  set_target_properties(utils-shared PROPERTIES
    PUBLIC_HEADER "${HEADERS}"
    OUTPUT_NAME "linqutils")
  install(TARGETS utils-shared
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
