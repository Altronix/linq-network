#--------------------------------------------------------------------------
# Build test-linq
#--------------------------------------------------------------------------

find_package(Threads)

add_executable(test-linq main.c)

string(CONCAT wrap_flags "-Wl,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_recv,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_bind,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_connect,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_recv")

target_link_libraries(test-linq linq-c zmq czmq mock-czmq cmocka)
set_target_properties(test-linq PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-linq COMMAND test-linq)
install(TARGETS test-linq DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
