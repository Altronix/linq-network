# --------------------------------------------------------------------------
# Build test-device
# --------------------------------------------------------------------------

find_package(Threads)

add_executable(test-device main.c)

target_link_libraries(test-device
                      atx-net-test-static
                      ${LIBCZMQ_STATIC_LIBS}
                      helpers
                      cmocka)

mock_czmq_linker_flags(wrap_flags)
set_target_properties(test-device PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-device COMMAND test-device)
install(TARGETS test-device DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
