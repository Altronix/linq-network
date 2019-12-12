add_library(helpers helpers.c helpers.h)
target_link_libraries(helpers
                      linq-netw-test
		      mock-czmq
		      stdc++
		      uuid)
target_include_directories(helpers PUBLIC .)
