# --------------------------------------------------------------------------
# Build test-devices
# --------------------------------------------------------------------------
add_executable(test-devices main.c)
target_link_libraries(test-devices
                      network-test-static
                      mock-utils
                      cmocka)

mock_utils_linker_flags(utils_flags)
set(wrap_flags "${sqlite_link_flags} ${utils_flags}")
set_target_properties(test-devices PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-devices COMMAND test-devices)
install(TARGETS test-devices DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
