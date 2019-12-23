# --------------------------------------------------------------------------
# Build test-routes
# --------------------------------------------------------------------------
add_executable(test-routes main.c)
target_link_libraries(test-routes
                      linq-netw-test
		      czmq
                      helpers
		      cmocka)

mock_czmq_linker_flags(czmq_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
set(wrap_flags "${czmq_link_flags} ${sqlite_link_flags}")
set_target_properties(test-routes PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-routes COMMAND test-routes)
install(TARGETS test-routes DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
