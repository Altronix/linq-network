set(SOURCES
    linq_network.c
    device.c
    node.c
    zmtp.c)
  set(HEADERS
    ../include/altronix/linq_network.h
    linq_network_internal.h
    log.h
    device.h
    node.h
    zmtp.h)

set(INSTALL_HEADERS ../include/altronix/linq_network.h)
set(INCS ../include)
set(LIBS ${CMAKE_THREAD_LIBS_INIT})
set(LIBS_STATIC 
  ${JSMN_WEB_TOKENS_STATIC_LIBS}
  ${LIBZMQ_STATIC_LIBS}
  linq-utils-static)
set(LIBS_SHARED 
  ${JSMN_WEB_TOKENS_SHARED_LIBS}
  ${LIBZMQ_SHARED_LIBS}
  linq-utils-shared)

append_log_level_compiler_flags(definitions)

if (WITH_SQLITE)
  MESSAGE(STATUS "SQlite Enabled")
  list(APPEND SOURCES 
    ./sqlite3.c 
    ./mongoose.c
    ./http.c
    ./http_auth.c
    ./routes/route_users.c
    ./routes/route_alerts.c
    ./routes/route_devices.c
    ./routes/route_proxy.c
    ./database/database.c)
  list(APPEND HEADERS
    ./sqlite3.h
    ./mongoose.h
    ./http.h
    ./http_auth.h
    ./http_auth_unsafe.h # TODO remove
    ./routes/routes.h
    ./database/database.h)
  append_mongoose_compiler_flags(definitions)
  append_sqlite_compiler_flags(definitions)
  list(APPEND LIBS_STATIC ${TLS_STATIC_LIBS})
  list(APPEND LIBS_SHARED ${TLS_SHARED_LIBS})
else()
  MESSAGE(STATUS "SQlite Disabled")
endif()

if(NOT MSVC)
  list(APPEND LIBS rt m stdc++ uuid dl)
else()
  list(APPEND definitions "-DCZMQ_STATIC")
  list(APPEND definitions "-DZMQ_STATIC")
  list(APPEND LIBS ws2_32 iphlpapi wldap32 uuid Rpcrt4)
endif()

if(ENABLE_TESTING)
  add_library(linq-network-test-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-network-test-static ${LIBS_STATIC} ${LIBS})
  target_compile_definitions(linq-network-test-static PUBLIC ${definitions})
  target_compile_definitions(linq-network-test-static PRIVATE "-DTESTING")
  target_include_directories(linq-network-test-static PUBLIC ${INCS})
  target_include_directories(linq-network-test-static PUBLIC ../include)
  target_include_directories(linq-network-test-static PUBLIC ./)
endif()

if(BUILD_STATIC)
  add_library(linq-network-static STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-network-static ${LIBS_STATIC} ${LIBS})
  target_compile_definitions(linq-network-static PUBLIC ${definitions})
  target_include_directories(linq-network-static PUBLIC ${INCS})
  target_include_directories(linq-network-static PRIVATE ./)
  set_target_properties(linq-network-static PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-network")
  install(TARGETS linq-network-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(linq-network SHARED ${SOURCES} ${HEADERS})
  if(MSVC)
    #TODO - https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows/41618677
    target_link_libraries(linq-network ${LIBS_STATIC} ${LIBS})
  else()
    target_link_libraries(linq-network ${LIBS_SHARED} ${LIBS})
  endif()
  target_compile_definitions(linq-network PUBLIC ${definitions})
  target_include_directories(linq-network PUBLIC ${INCS})
  target_include_directories(linq-network PRIVATE ./)
  set_target_properties(linq-network PROPERTIES PUBLIC_HEADER "${INSTALL_HEADERS}")
  install(TARGETS linq-network
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
