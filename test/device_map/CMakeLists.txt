#--------------------------------------------------------------------------
# Build test-device-map
#--------------------------------------------------------------------------

find_package(Threads)

add_executable(test-device-map main.c)

target_link_libraries(test-device-map linq-c-test helpers cmocka)
add_test(NAME test-device-map COMMAND test-device-map)
install(TARGETS test-device-map DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
