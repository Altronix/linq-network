# --------------------------------------------------------------------------
# Build test-json
# --------------------------------------------------------------------------
add_executable(test-json main.c)
target_link_libraries(test-json
                      network-test-static
                      mock-utils
		      cmocka)

add_test(NAME test-json COMMAND test-json)
install(TARGETS test-json DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
