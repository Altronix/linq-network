# --------------------------------------------------------------------------
# Build test-routes
# --------------------------------------------------------------------------
add_executable(test-routes 
                      main.c
                      main.h
                      alerts.c
                      alerts.h
                      devices.c
                      devices.h
                      login.c
                      login.h
                      proxy.c
                      proxy.h
                      create_admin.c
                      create_admin.h)
target_link_libraries(test-routes
                      linq-http-test
                      mock-utils
                      ${LIBZMQ_STATIC_LIBS}
                      helpers
		      cmocka)

mock_czmq_linker_flags(czmq_flags)
mock_mongoose_linker_flags(mongoose_flags)
mock_sqlite_linker_flags(sqlite_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${czmq_flags} ${mongoose_flags} ${sqlite_flags} ${utils_flags}")
set_target_properties(test-routes PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-routes COMMAND test-routes)
install(TARGETS test-routes DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
