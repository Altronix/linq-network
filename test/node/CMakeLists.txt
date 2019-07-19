#--------------------------------------------------------------------------
# Build test-node
#--------------------------------------------------------------------------

find_package(Threads)

add_executable(test-node main.c)

string(CONCAT wrap_flags "-Wl,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmq_poll,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_recv,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_bind,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_resolve,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_connect,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_recv")

target_link_libraries(test-node linq-c-test zmq czmq helpers mock-czmq cmocka)
set_target_properties(test-node PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-node COMMAND test-node)
install(TARGETS test-node DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
