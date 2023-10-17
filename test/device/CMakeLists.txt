# --------------------------------------------------------------------------
# Build test-device
# --------------------------------------------------------------------------
add_executable(test-device main.c)
target_link_libraries(test-device
                      network-test-static
                      mock-utils
                      helpers
                      cmocka)

mock_zmq_linker_flags(zmq_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${zmq_flags} ${sqlite_link_flags} ${utils_flags}")
set_target_properties(test-device PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-device COMMAND test-device)
install(TARGETS test-device DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
