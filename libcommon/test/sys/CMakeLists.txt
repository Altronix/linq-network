append_mock_file_linker_flags(wrap_flags)

add_executable(test-sys main.c)
set_target_properties(test-sys PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-sys cmocka mock-file common-static)
add_test(NAME test-sys COMMAND test-sys)
install(TARGETS test-sys DESTINATION bin)
