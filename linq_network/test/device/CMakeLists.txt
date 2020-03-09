# --------------------------------------------------------------------------
# Build test-device
# --------------------------------------------------------------------------

add_executable(test-device main.c)

target_link_libraries(test-device
                      linq-network-test-static
                      mock-utils
                      ${LIBZMQ_STATIC_LIBS}
                      helpers
                      cmocka)

mock_czmq_linker_flags(czmq_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${czmq_flags} ${utils_flags}")
set_target_properties(test-device PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-device COMMAND test-device)
install(TARGETS test-device DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
