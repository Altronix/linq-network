# --------------------------------------------------------------------------
# Lib of mock functions for test
# --------------------------------------------------------------------------
add_library(mock-czmq mock_zmsg.c mock_zsock.c mock_zpoll.c)
target_link_libraries(mock-czmq
                      linq-netw-test
                      czmq
                      zmq)
target_include_directories(mock-czmq PUBLIC .)

string(CONCAT wrap_flags "-Wl,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmq_poll,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_recv,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_new_router,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_new_dealer,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_type_str,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_destroy,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_new_router_checked,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_new_dealer_checked,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_destroy_checked,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_bind,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_resolve,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_connect,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_recv")

add_executable(test-mocks main.c)
set_target_properties(test-mocks PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-mocks
                      linq-netw-test
                      czmq
                      helpers
                      mock-czmq
                      cmocka
                      stdc++) #linking stdc++ OK since (tests linux only)
add_test(NAME test-mocks COMMAND test-mocks)
install(TARGETS test-mocks DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
