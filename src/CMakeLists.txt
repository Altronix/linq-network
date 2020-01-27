set(SOURCES
    base64.c
    atx_net.c
    device.c
    node.c
    jsmn_helpers.c
    sys.c
    zmtp.c)
  set(HEADERS
    ../include/altronix/atx_net.h
    base64.h
    containers.h
    jsmn.h
    jsmn_helpers.h
    atx_net_internal.h
    log.h
    device.h
    node.h
    zmtp.h)

set(INSTALL_HEADERS ../include/altronix/atx_net.h)
set(INCS ../include)
set(LIBS ${CMAKE_THREAD_LIBS_INIT})
set(LIBS_STATIC ${LIBCZMQ_STATIC_LIBS} ${LIBZMQ_STATIC_LIBS})
set(LIBS_SHARED ${LIBCZMQ_SHARED_LIBS} ${LIBZMQ_SHARED_LIBS})
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
  list(APPEND LIBS OpenSSL::SSL OpenSSL::Crypto)
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
  add_library(atx-net-test-static STATIC
              ${SOURCES}
              ${HEADERS}
              ${SOURCES_SYS_MOCK}
              ${HEADERS_SYS_MOCK})
  target_link_libraries(atx-net-test-static
    ${LIBS_STATIC}
    ${LIBS})
  target_compile_definitions(atx-net-test-static PUBLIC ${definitions})
  target_include_directories(atx-net-test-static PUBLIC ${INCS})
  target_include_directories(atx-net-test-static PUBLIC ../include)
  target_include_directories(atx-net-test-static PUBLIC ./sys/mock)
  target_include_directories(atx-net-test-static PUBLIC ./)
endif()

if(BUILD_STATIC)
  add_library(atx-net-static STATIC
    ${SOURCES}
    ${HEADERS}
    ${SOURCES_SYS}
    ${HEADERS_SYS})
  target_link_libraries(atx-net-static
    ${LIBS_STATIC}
    ${LIBS})
  target_compile_definitions(atx-net-static PUBLIC ${definitions})
  target_include_directories(atx-net-static PUBLIC ${INCS})
  target_include_directories(atx-net-static PRIVATE sys/unix)
  target_include_directories(atx-net-static PRIVATE ./)
  set_target_properties(atx-net-static PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "atx-net")
  install(TARGETS atx-net-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(atx-net SHARED
    ${SOURCES}
    ${HEADERS}
    ${SOURCES_SYS}
    ${HEADERS_SYS})
  if(MSVC)
    #TODO - https://stackoverflow.com/questions/33062728/cmake-link-shared-library-on-windows/41618677
    target_link_libraries(atx-net
      ${LIBS_STATIC}
      ${LIBS})
  else()
    target_link_libraries(atx-net
      ${LIBS_SHARED}
      ${LIBS})
  endif()
  target_compile_definitions(atx-net PUBLIC ${definitions})
  target_include_directories(atx-net PUBLIC ${INCS})
  target_include_directories(atx-net PRIVATE sys/unix)
  target_include_directories(atx-net PRIVATE ./)
  set_target_properties(atx-net PROPERTIES PUBLIC_HEADER "${INSTALL_HEADERS}")
  install(TARGETS atx-net
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
