# --------------------------------------------------------------------------
# Lib of mock functions for test
# --------------------------------------------------------------------------
add_library(mock-utils mock_utils.c mock_utils.h)

target_link_libraries(mock-utils
  cmocka
  stdc++)

target_include_directories(mock-utils PUBLIC .)

function(mock_utils_linker_flags dst)
  string(CONCAT wrap_flags "-Wl,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=sys_tick,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=sys_unix")
  set(${dst} ${wrap_flags} PARENT_SCOPE)
endfunction()

# mock_utils_linker_flags(wrap_flags)
# append_utils_compiler_flags(definitions)

# add_executable(test-mock-utils main.c)
# set_target_properties(test-mock-utils PROPERTIES LINK_FLAGS ${wrap_flags})
# target_compile_definitions(test-mock-utils PRIVATE ${definitions})
# target_link_libraries(test-mock-utils
#                       mock-utils
#                       cmocka
#                       helpers)
# add_test(NAME test-mock-utils COMMAND test-mock-utils)
# install(TARGETS test-mock-utils DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
