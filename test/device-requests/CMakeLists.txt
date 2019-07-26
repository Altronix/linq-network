#--------------------------------------------------------------------------
# Build test-device-requests
#--------------------------------------------------------------------------

find_package(Threads)

add_executable(test-device-requests main.c)

target_link_libraries(test-device-requests linq-c-test helpers cmocka)
add_test(NAME test-device-requests COMMAND test-device-requests)
install(TARGETS test-device-requests DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
