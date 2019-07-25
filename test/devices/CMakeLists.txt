#--------------------------------------------------------------------------
# Build test-devices
#--------------------------------------------------------------------------

find_package(Threads)

add_executable(test-devices main.c)

target_link_libraries(test-devices linq-c-test helpers cmocka)
add_test(NAME test-devices COMMAND test-devices)
install(TARGETS test-devices DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
