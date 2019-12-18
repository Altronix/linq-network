# --------------------------------------------------------------------------
# Build test-linq-netw
# --------------------------------------------------------------------------
add_executable(test-linq-netw main.c)
target_link_libraries(test-linq-netw
                      linq-netw-test
		      czmq
                      helpers
		      cmocka)

# TODO append mock_sqlite_linker_flags() (seems to break test)
mock_czmq_linker_flags(wrap_flags)
set_target_properties(test-linq-netw PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-linq-netw COMMAND test-linq-netw)
install(TARGETS test-linq-netw DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
