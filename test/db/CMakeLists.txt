# --------------------------------------------------------------------------
# Build test-db
# --------------------------------------------------------------------------
add_executable(test-db main.c)
target_link_libraries(test-db
                      linq-network-test-static
                      mock-sqlite
                      helpers
		      cmocka)

mock_sqlite_linker_flags(wrap_flags)
set_target_properties(test-db PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-db COMMAND test-db)
install(TARGETS test-db DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
