# --------------------------------------------------------------------------
# Build test-callbacks
# --------------------------------------------------------------------------
add_executable(test-callbacks main.c)
target_link_libraries(test-callbacks
                      network-test-static
                      mock-utils
                      helpers
                      cmocka)

mock_mongoose_linker_flags(mongoose_link_flags)
mock_czmq_linker_flags(czmq_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${czmq_flags} ${sqlite_link_flags} ${utils_flags}")
set_target_properties(test-callbacks PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-callbacks COMMAND test-callbacks)
install(TARGETS test-callbacks DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
