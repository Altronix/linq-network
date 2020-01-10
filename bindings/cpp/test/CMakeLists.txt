add_executable(binding-linq-netw-cpp-test main.cpp)

mock_czmq_linker_flags(czmq_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
set(wrap_flags "${czmq_link_flags} ${sqlite_link_flags}")
target_link_libraries(binding-linq-netw-cpp-test
                      linq-netw-cpp-binding
                      linq-netw-test-static
                      mock-czmq
                      zmq
                      czmq
                      helpers
                      cmocka)
set_target_properties(binding-linq-netw-cpp-test PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(binding-linq-netw-cpp-test PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME binding-linq-netw-cpp-test COMMAND binding-linq-netw-cpp-test)
