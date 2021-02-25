add_library(helpers 
  helpers.c 
  helpers.h)

target_link_libraries(helpers
  network-static
  mock-zmq
  stdc++
  uuid)

target_include_directories(helpers PUBLIC .)
