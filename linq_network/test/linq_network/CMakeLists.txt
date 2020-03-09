# --------------------------------------------------------------------------
# Build test-linq-network
# --------------------------------------------------------------------------
add_executable(test-linq-network main.c)
target_link_libraries(test-linq-network
                      linq-network-test-static
                      mock-utils
                      ${LIBCZMQ_STATIC_LIBS}
                      helpers
		      cmocka)

mock_czmq_linker_flags(czmq_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${czmq_link_flags} ${sqlite_link_flags} ${utils_flags}")
set_target_properties(test-linq-network PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-linq-network COMMAND test-linq-network)
install(TARGETS test-linq-network DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
