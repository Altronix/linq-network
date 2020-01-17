# --------------------------------------------------------------------------
# Build test-jsmn
# --------------------------------------------------------------------------
add_executable(test-jsmn main.c)
target_link_libraries(test-jsmn
                      atx-net-test-static
                      helpers
		      cmocka)

add_test(NAME test-jsmn COMMAND test-jsmn)
install(TARGETS test-jsmn DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
