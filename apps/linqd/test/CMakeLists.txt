add_executable(test-linqd-parse-config main.c)
target_link_libraries(test-linqd-parse-config 
  cmocka
  mock-mongoose
  mock-file
  mock-utils
  atx-linqd-lib
  network-test-static)
  
mock_utils_linker_flags(wrap_flags)
mock_mongoose_linker_flags(wrap_flags)
append_mock_file_linker_flags(wrap_flags)
list(APPEND wrap_flags "-Wl,--wrap=sys_config_dir,--wrap=sys_open")
set_target_properties(test-linqd-parse-config PROPERTIES LINK_FLAGS "${wrap_flags}")
add_test(NAME test-linqd-parse-config COMMAND test-linqd-parse-config)
install(TARGETS test-linqd-parse-config DESTINATION bin)
