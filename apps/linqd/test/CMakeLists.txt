add_executable(test-linqd-parse-config main.c)
target_link_libraries(test-linqd-parse-config 
  atx-linqd-lib
  network-test-static 
  mock-utils 
  cmocka)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${utils_flags}")
set_target_properties(test-linqd-parse-config PROPERTIES LINK_FLAGS ${wrap_flags})
add_test(NAME test-linqd-parse-config COMMAND test-linqd-parse-config)
install(TARGETS test-linqd-parse-config DESTINATION bin)
