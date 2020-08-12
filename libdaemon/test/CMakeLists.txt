# --------------------------------------------------------------------------
# Build test-linqd
# --------------------------------------------------------------------------
add_executable(test-linqd main.c)
target_link_libraries(test-linqd
                      linq-daemon-static
                      helpers
		      cmocka)

mock_mongoose_linker_flags(mongoose_link_flags)
mock_czmq_linker_flags(czmq_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${czmq_link_flags} ${sqlite_link_flags} ${utils_flags} ${mongoose_link_flags}")
set_target_properties(test-linqd PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-linqd COMMAND test-linqd)
install(TARGETS test-linqd DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
