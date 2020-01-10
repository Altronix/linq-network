add_library(helpers 
  helpers.c 
  helpers.h)

target_link_libraries(helpers
  linq-netw-test-static
  mock-czmq
  mock-mongoose
  mock-sqlite
  stdc++
  uuid)

target_include_directories(helpers PUBLIC .)
