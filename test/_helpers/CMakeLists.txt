add_library(helpers helpers.c helpers.h)
target_link_libraries(helpers
                      linq-netw-test
                      czmq
                      zmq
                      cmocka)
target_include_directories(helpers PUBLIC .)
