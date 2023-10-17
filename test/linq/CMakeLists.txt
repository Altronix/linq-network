# --------------------------------------------------------------------------
# Build test-linq
# --------------------------------------------------------------------------
add_executable(test-linq main.c)
target_link_libraries(test-linq
                      network-test-static
                      mock-utils
                      helpers
		      cmocka)

mock_zmq_linker_flags(zmq_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${zmq_flags} ${utils_flags}")
set_target_properties(test-linq PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-linq COMMAND test-linq)
install(TARGETS test-linq DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
