add_executable(binding-linq-network-cpp-test main.cpp)

mock_czmq_linker_flags(czmq_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
set(wrap_flags "${czmq_link_flags} ${sqlite_link_flags}")
target_link_libraries(binding-linq-network-cpp-test
                      linq-network-cpp-binding
                      linq-network-test-static
                      mock-czmq
                      ${LIBCZMQ_STATIC_LIBS}
                      ${LIBZMQ_STATIC_LIBS}
                      helpers
                      cmocka)
set_target_properties(binding-linq-network-cpp-test PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(binding-linq-network-cpp-test PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME binding-linq-network-cpp-test COMMAND binding-linq-network-cpp-test)
