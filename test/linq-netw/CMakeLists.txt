# --------------------------------------------------------------------------
# Build test-linq-netw
# --------------------------------------------------------------------------
add_executable(test-linq-netw main.c)
target_link_libraries(test-linq-netw
                      linq-netw-test-static
                      ${LIBCZMQ_STATIC_LIBS}
                      helpers
		      cmocka)

mock_czmq_linker_flags(czmq_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
set(wrap_flags "${czmq_link_flags} ${sqlite_link_flags}")
set_target_properties(test-linq-netw PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-linq-netw COMMAND test-linq-netw)
install(TARGETS test-linq-netw DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
