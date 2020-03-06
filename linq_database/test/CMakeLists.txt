# --------------------------------------------------------------------------
# Build test-database
# --------------------------------------------------------------------------
add_executable(test-database main.c)
target_link_libraries(test-database
                      linq-database-static
                      mock-sqlite
                      mock-utils
                      helpers
		      cmocka)

mock_sqlite_linker_flags(sqlite_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${sqlite_flags} ${utils_flags}")
set_target_properties(test-database PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-database COMMAND test-database)
install(TARGETS test-database DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
