add_executable(binding-atx-net-cpp-test main.cpp)

mock_czmq_linker_flags(czmq_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
set(wrap_flags "${czmq_link_flags} ${sqlite_link_flags}")
target_link_libraries(binding-atx-net-cpp-test
                      atx-net-cpp-binding
                      atx-net-test-static
                      mock-czmq
                      ${LIBCZMQ_STATIC_LIBS}
                      ${LIBZMQ_STATIC_LIBS}
                      helpers
                      cmocka)
set_target_properties(binding-atx-net-cpp-test PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(binding-atx-net-cpp-test PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME binding-atx-net-cpp-test COMMAND binding-atx-net-cpp-test)
