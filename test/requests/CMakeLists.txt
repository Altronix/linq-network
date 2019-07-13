#--------------------------------------------------------------------------
# Build test-requests
#--------------------------------------------------------------------------

find_package(Threads)

add_executable(test-requests main.c)

target_link_libraries(test-requests linq-c-test helpers cmocka)
add_test(NAME test-requests COMMAND test-requests)
install(TARGETS test-requests DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
