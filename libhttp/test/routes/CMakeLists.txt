# --------------------------------------------------------------------------
# Build test-http-routes
# --------------------------------------------------------------------------
add_executable(test-http-routes main.c)
target_link_libraries(test-http-routes
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
set_target_properties(test-http-routes PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-http-routes COMMAND test-http-routes)
install(TARGETS test-http-routes DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
