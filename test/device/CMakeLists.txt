#--------------------------------------------------------------------------
# Build test-device
#--------------------------------------------------------------------------

find_package(Threads)

add_executable(test-device main.c)

target_link_libraries(test-device linq-c-test mock-czmq zmq czmq helpers cmocka)
add_test(NAME test-device COMMAND test-device)
install(TARGETS test-device DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
