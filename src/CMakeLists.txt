set(SOURCES
    base64.c
    linq_netw.c
    device.c
    node.c
    jsmn_helpers.c
    sys.c
    zmtp.c)
  set(HEADERS
    ../include/altronix/linq_netw.h
    base64.h
    containers.h
    jsmn_helpers.h
    linq_netw_internal.h
    log.h
    device.h
    node.h
    zmtp.h)

set(INSTALL_HEADERS ../include/altronix/linq_netw.h)
set(LIBS czmq zmq jsmn klib)
set(INCS ../include)
set(SOURCES_SYS_MOCK sys/mock/sys.c)
set(HEADERS_SYS_MOCK sys/mock/sys.h)

append_log_level_compiler_flags(definitions)

if (WITH_SQLITE)
  MESSAGE(STATUS "SQlite Enabled")
  list(APPEND SOURCES 
    ./sqlite3.c 
    ./mongoose.c
    ./http.c
    ./routes/route_alerts.c
    ./routes/route_devices.c
    ./routes/route_proxy.c
    ./database/database.c)
  list(APPEND HEADERS
    ./sqlite3.h
    ./mongoose.h
    ./http.h
    ./routes/routes.h
    ./database/database.h)
  append_mongoose_compiler_flags(definitions)
  append_sqlite_compiler_flags(definitions)
else()
  MESSAGE(STATUS "SQlite Disabled")
endif()

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
  list(APPEND SOURCES_SYS sys/unix/sys.c)
  list(APPEND HEADERS_SYS sys/unix/sys.h)
else()
  list(APPEND definitions "-DCZMQ_STATIC")
  list(APPEND definitions "-DZMQ_STATIC")
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
  list(APPEND INCS ./sys/win)
  list(APPEND SOURCES_SYS sys/win/sys.c)
  list(APPEND HEADERS_SYS sys/win/sys.h)
endif()

if(ENABLE_TESTING)
  add_library(linq-netw-test-static STATIC
              ${SOURCES}
              ${HEADERS}
              ${SOURCES_SYS_MOCK}
              ${HEADERS_SYS_MOCK})
  target_link_libraries(linq-netw-test-static
    ${LIBS}
    ${CMAKE_THREAD_LIBS_INIT})
  target_compile_definitions(linq-netw-test-static PUBLIC ${definitions})
  target_include_directories(linq-netw-test-static PUBLIC ${INCS})
  target_include_directories(linq-netw-test-static PUBLIC ../include)
  target_include_directories(linq-netw-test-static PUBLIC ./sys/mock)
  target_include_directories(linq-netw-test-static PUBLIC ./)
endif()

if(BUILD_STATIC)
  add_library(linq-netw-static STATIC
    ${SOURCES}
    ${HEADERS}
    ${SOURCES_SYS}
    ${HEADERS_SYS})
  target_link_libraries(linq-netw-static
    ${LIBS}
    ${CMAKE_THREAD_LIBS_INIT})
  target_compile_definitions(linq-netw-static PUBLIC ${definitions})
  target_include_directories(linq-netw-static PUBLIC ${INCS})
  target_include_directories(linq-netw-static PRIVATE sys/unix)
  target_include_directories(linq-netw-static PRIVATE ./)
  set_target_properties(linq-netw-static PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-netw")
  install(TARGETS linq-netw-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(linq-netw SHARED
    ${SOURCES}
    ${HEADERS}
    ${SOURCES_SYS}
    ${HEADERS_SYS})
  target_link_libraries(linq-netw ${LIBS} ${CMAKE_THREAD_LIBS_INIT})
  target_compile_definitions(linq-netw PUBLIC ${definitions})
  target_include_directories(linq-netw PUBLIC ${INCS})
  target_include_directories(linq-netw PRIVATE sys/unix)
  target_include_directories(linq-netw PRIVATE ./)
  set_target_properties(linq-netw PROPERTIES PUBLIC_HEADER "${INSTALL_HEADERS}")
  install(TARGETS linq-netw
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
