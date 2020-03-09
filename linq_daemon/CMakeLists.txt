add_library(linqd-library linq_daemon.c linq_daemon.h)
target_link_libraries(linqd-library 
  linq-network-static
  linq-database-static
  linq-http-static
  linq-utils-static
  ${JSMN_WEB_TOKENS_STATIC_LIBS}
  ${LIBZMQ_STATIC_LIBS}
  ${TLS_STATIC_LIBS})

add_executable(linqd main.c)
target_link_libraries(linqd linqd-library)
install(TARGETS linqd DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
