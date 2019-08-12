# --------------------------------------------------------------------------
# Build test-linq
# --------------------------------------------------------------------------

find_package(Threads)

add_executable(test-linq main.c)

string(CONCAT wrap_flags "-Wl,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmq_poll,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_recv,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_new_router,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_new_dealer,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_destroy,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_new_router_checked,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_new_dealer_checked,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_destroy_checked,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_bind,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_resolve,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_connect,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_recv")

target_link_libraries(test-linq
                      linq-c-test
                      zmq
                      czmq
                      helpers
                      mock-czmq
                      cmocka)
set_target_properties(test-linq PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-linq COMMAND test-linq)
install(TARGETS test-linq DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
