add_executable(test-rlp main.c)
target_link_libraries(test-rlp cmocka network-test-static)
add_test(NAME test-rlp COMMAND test-rlp)
install(TARGETS test-rlp DESTINATION bin)
