# Main source files
set(SOURCES config.c device.c base64.c rlp.c json.c)
set(HEADERS 
  common.h
  config.h
  device.h
  base64.h 
  json.h 
  jsmn.h 
  log.h
  containers.h
  klib/khash.h 
  klib/klist.h)
set(INSTALL_HEADERS common.h)

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
  list(APPEND INCS ./sys/unix)
  list(APPEND SOURCES sys/unix/sys.c)
  list(APPEND HEADERS sys/unix/sys.h)
  list(APPEND INSTALL_HEADERS sys/unix/sys.h)
else()
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
  list(APPEND INCS ./sys/win)
  list(APPEND SOURCES sys/win/sys.c)
  list(APPEND HEADERS sys/win/sys.h)
  list(APPEND INSTALL_HEADERS sys/win/sys.h)
endif()

list(APPEND INCS ./)

# Build a static library
if(BUILD_STATIC)
  add_library(common-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(common-static ${LIBS})
  target_compile_definitions(common-static PUBLIC "LINQ_STATIC")
  target_compile_definitions(common-static PUBLIC "RLP_STATIC")
  target_include_directories(common-static PUBLIC ${INCS})
  set_target_properties(common-static PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqcommon")
  install(TARGETS common-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

# Build a shared library
if(BUILD_SHARED)
  add_library(common-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(common-shared ${LIBS})
  target_compile_definitions(common-shared PUBLIC "DLL_EXPORT")
  target_include_directories(common-shared PUBLIC ${INCS})
  set_target_properties(common-shared PROPERTIES
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqcommon")
  install(TARGETS common-shared
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
