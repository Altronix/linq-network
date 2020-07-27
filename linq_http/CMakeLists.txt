set(SOURCES 
  ./mongoose.c
  ./http.c
  ./http_auth.c
  ./route_alerts.c
  ./route_devices.c
  ./route_proxy.c
  ./route_users.c)
set(HEADERS
  ./mongoose.h
  ./http.h
  ./http_auth.h
  ./http_auth_unsafe.h
  ./routes.h)

append_log_level_compiler_flags(definitions)
append_mongoose_compiler_flags(definitions)
append_sqlite_compiler_flags(definitions)
set(INSTALL_HEADERS http.h)
list(APPEND definitions "-DCZMQ_STATIC")
list(APPEND definitions "-DZMQ_STATIC")

set(LIBS_STATIC
  linq-network-static
  linq-database-static
  linq-utils-static
  ${JSMN_WEB_TOKENS_STATIC_LIBS}
  ${LIBZMQ_STATIC_LIBS}
  ${TLS_STATIC_LIBS})

set(LIBS_SHARED
  linq-network-shared
  linq-database-shared
  linq-utils-shared
  ${JSMN_WEB_TOKENS_SHARED_LIBS}
  ${LIBZMQ_SHARED_LIBS}
  ${TLS_SHARED_LIBS})

if(ENABLE_TESTING)
  add_library(linq-http-test STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(linq-http-test ${LIBS_STATIC})
  target_compile_definitions(linq-http-test PRIVATE "-DTESTING")
  target_compile_definitions(linq-http-test PUBLIC ${definitions})
  target_include_directories(linq-http-test PUBLIC ./)
  set_target_properties(linq-http-test PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-http-test")
  install(TARGETS linq-http-test
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_STATIC)
  # for some reason windows is trying to link with shared if it exists
  # cmake seems to prefer the shared target even when explicitly linking with
  # the static target.
  # (For some reason only linq_http is doing that....)
  add_library(linq-http-static ${SOURCES} ${HEADERS})
  target_link_libraries(linq-http-static ${LIBS_STATIC})
  target_compile_definitions(linq-http-static PUBLIC ${definitions})
  target_include_directories(linq-http-static PUBLIC ./)
  set_target_properties(linq-http-static PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-http")
  install(TARGETS linq-http-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(linq-http-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(linq-http-shared ${LIBS_SHARED})
  target_compile_definitions(linq-http-shared PUBLIC ${definitions})
  target_include_directories(linq-http-shared PUBLIC ./)
  target_include_directories(linq-http-shared PUBLIC ../include)
  set_target_properties(linq-http-shared PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linq-http")
  install(TARGETS linq-http-shared
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
