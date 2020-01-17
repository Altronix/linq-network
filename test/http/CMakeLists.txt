# --------------------------------------------------------------------------
# Build test-http
# --------------------------------------------------------------------------
add_executable(test-http main.c)
target_link_libraries(test-http
                      atx-net-test-static
                      helpers
		      cmocka)

mock_mongoose_linker_flags(wrap_flags)
set_target_properties(test-http PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-http COMMAND test-http)
install(TARGETS test-http DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
