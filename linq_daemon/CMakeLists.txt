add_library(linq-daemon-static linq_daemon.c linq_daemon.h)
target_link_libraries(linq-daemon-static 
  linq-network-static
  linq-database-static
  linq-http-static
  linq-utils-static
  ${JSMN_WEB_TOKENS_STATIC_LIBS}
  ${LIBZMQ_STATIC_LIBS}
  ${TLS_STATIC_LIBS})
  
set_target_properties(linq-daemon-static PROPERTIES 
  PUBLIC_HEADER "linq_daemon.h")
target_include_directories(linq-daemon-static PUBLIC ./)

add_executable(linqd main.c)
target_link_libraries(linqd linq-http-static linq-daemon-static)
install(TARGETS linqd RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
