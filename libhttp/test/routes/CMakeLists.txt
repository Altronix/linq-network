# --------------------------------------------------------------------------
# Build test-http-users
# --------------------------------------------------------------------------
add_executable(test-http-users main.c)
target_link_libraries(test-http-users
                      http-test
                      database-static
                      mock-utils
                      ${LIBZMQ_STATIC_LIBS}
                      helpers
		      cmocka)

mock_mongoose_linker_flags(mongoose_flags)
mock_sqlite_linker_flags(sqlite_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${mongoose_flags} ${sqlite_flags} ${utils_flags}")
set_target_properties(test-http-users PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-http-users COMMAND test-http-users)
install(TARGETS test-http-users DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
