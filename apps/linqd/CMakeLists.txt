add_library(atx-linqd-lib STATIC 
  callbacks.c
  callbacks.h
  config.c 
  config.h 
  jsmn.h
  json.c
  json.h
  log.c
  log.h
  sys.c
  sys.h)
target_link_libraries(atx-linqd-lib network-static)

add_executable(atx-linqd main.c)
target_link_libraries(atx-linqd atx-linqd-lib network-static)
install(TARGETS atx-linqd RUNTIME DESTINATION bin)
