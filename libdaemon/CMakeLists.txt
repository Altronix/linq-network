add_library(daemon-static daemon.c daemon.h)
list(APPEND LIBS 
  network-static
  database-static
  http-static
  utils-static
  ${JSMN_WEB_TOKENS_LIBS}
  ${ZMQ_LIBS}
  ${TLS_LIBS})
if(NOT MSVC)
  list(APPEND LIBS stdc++)
else()
endif()
target_link_libraries(daemon-static  ${LIBS})
target_compile_definitions(daemon-static PUBLIC "LINQ_DAEMON_STATIC")
  
set_target_properties(daemon-static PROPERTIES 
  PUBLIC_HEADER "daemon.h")
target_include_directories(daemon-static PUBLIC ./)
