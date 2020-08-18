set(SOURCES 
  ./mongoose.c
  ./http.c
  ./http_users.c
  ./http_auth.c)
set(HEADERS
  ./mongoose.h
  ./http.h
  ./http_users.h
  ./http_auth.h
  ./http_auth_unsafe.h)

append_log_level_compiler_flags(definitions)
append_mongoose_compiler_flags(definitions)
append_sqlite_compiler_flags(definitions)
set(INSTALL_HEADERS http.h)
list(APPEND definitions "-DCZMQ_STATIC")
list(APPEND definitions "-DZMQ_STATIC")
list(APPEND definitions "-DBUILD_LINQD")

set(LIBS
  database-static
  common-static
  ${JSMN_WEB_TOKENS_LIBS}
  ${ZMQ_LIBS}
  ${TLS_LIBS})

if(ENABLE_TESTING)
  add_library(http-test STATIC ${SOURCES} ${HEADERS})
  target_link_libraries(http-test ${LIBS})
  target_compile_definitions(http-test PRIVATE "-DTESTING")
  target_compile_definitions(http-test PUBLIC 
    "LINQ_HTTP_STATIC" 
    ${definitions})
  target_include_directories(http-test PUBLIC ./)
  set_target_properties(http-test PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqhttp-test")
  install(TARGETS http-test
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_STATIC)
  add_library(http-static ${SOURCES} ${HEADERS})
  target_link_libraries(http-static ${LIBS})
  target_compile_definitions(http-static PUBLIC 
    "LINQ_HTTP_STATIC" 
    ${definitions})
  target_include_directories(http-static PUBLIC ./)
  set_target_properties(http-static PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqhttp")
  install(TARGETS http-static
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          PUBLIC_HEADER DESTINATION include/altronix)
endif()

if(BUILD_SHARED)
  add_library(http-shared SHARED ${SOURCES} ${HEADERS})
  target_link_libraries(http-shared ${LIBS})
  target_compile_definitions(http-shared PUBLIC 
    ${definitions}
    "DLL_EXPORT")
  target_include_directories(http-shared PUBLIC ./)
  target_include_directories(http-shared PUBLIC ../include)
  set_target_properties(http-shared PROPERTIES 
    PUBLIC_HEADER "${INSTALL_HEADERS}"
    OUTPUT_NAME "linqhttp")
  install(TARGETS http-shared
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib
          RUNTIME DESTINATION bin
          PUBLIC_HEADER DESTINATION include/altronix)
endif()
