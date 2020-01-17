# --------------------------------------------------------------------------
# Build test-atx-net
# --------------------------------------------------------------------------
add_executable(test-atx-net main.c)
target_link_libraries(test-atx-net
                      atx-net-test-static
                      ${LIBCZMQ_STATIC_LIBS}
                      helpers
		      cmocka)

mock_czmq_linker_flags(czmq_link_flags)
mock_sqlite_linker_flags(sqlite_link_flags)
set(wrap_flags "${czmq_link_flags} ${sqlite_link_flags}")
set_target_properties(test-atx-net PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-atx-net COMMAND test-atx-net)
install(TARGETS test-atx-net DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
