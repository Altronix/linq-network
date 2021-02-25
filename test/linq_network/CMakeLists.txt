# --------------------------------------------------------------------------
# Build test-network
# --------------------------------------------------------------------------
add_executable(test-network main.c)
target_link_libraries(test-network
                      network-test-static
                      mock-utils
                      helpers
		      cmocka)

mock_czmq_linker_flags(czmq_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${czmq_link_flags} ${sqlite_link_flags} ${utils_flags}")
set_target_properties(test-network PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-network COMMAND test-network)
install(TARGETS test-network DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
