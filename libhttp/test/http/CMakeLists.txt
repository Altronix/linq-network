# --------------------------------------------------------------------------
# Build test-http
# --------------------------------------------------------------------------
add_executable(test-http main.c)
target_link_libraries(test-http
                      http-test
                      mock-utils
                      helpers
		      cmocka)

mock_mongoose_linker_flags(mongoose_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${mongoose_link_flags} ${sqlite_link_flags} ${utils_flags}")
set_target_properties(test-http PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-http COMMAND test-http)
install(TARGETS test-http DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
