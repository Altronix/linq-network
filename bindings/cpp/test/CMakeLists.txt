add_executable(binding-linq-io-cpp-test main.cpp)

string(CONCAT wrap_flags "-Wl,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmq_poll,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_recv,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zmsg_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_bind,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_resolve,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_connect,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_send,")
string(CONCAT wrap_flags ${wrap_flags} "--wrap=zsock_recv")

target_link_libraries(binding-linq-io-cpp-test
                      linq-io-cpp-binding
                      linq-io-test
                      mock-czmq
                      zmq
                      czmq
                      helpers
                      cmocka)
set_target_properties(binding-linq-io-cpp-test PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(binding-linq-io-cpp-test PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME binding-linq-io-cpp-test COMMAND binding-linq-io-cpp-test)
