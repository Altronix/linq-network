add_executable(test-linqd-parse-config main.c)
target_link_libraries(test-linqd-parse-config 
  atx-linqd-lib
  network-test-static 
  cmocka
  mock-utils 
  mock-file)
message(STATUS "wrap_flags_0: ${wrap_flags}")
mock_utils_linker_flags(wrap_flags)
message(STATUS "wrap_flags_1: ${wrap_flags}")
append_mock_file_linker_flags(wrap_flags)
message(STATUS "wrap_flags_2: ${wrap_flags}")
set_target_properties(test-linqd-parse-config PROPERTIES LINK_FLAGS "${wrap_flags}")
add_test(NAME test-linqd-parse-config COMMAND test-linqd-parse-config)
install(TARGETS test-linqd-parse-config DESTINATION bin)
