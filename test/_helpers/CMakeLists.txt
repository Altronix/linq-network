add_library(helpers 
  helpers.c 
  helpers.h)

target_link_libraries(helpers
  mock-mongoose
  mock-sqlite
  mock-czmq
  mock-utils
  stdc++
  uuid)

target_include_directories(helpers PUBLIC .)
