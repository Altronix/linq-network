# --------------------------------------------------------------------------
# Lib of mock functions for test
# --------------------------------------------------------------------------
add_library(mock-utils mock_utils.c mock_utils.h)

target_link_libraries(mock-utils
  cmocka
  stdc++)

target_include_directories(mock-utils PUBLIC .)

function(mock_utils_linker_flags dst)
  string(CONCAT __arg "-Wl,")
  string(CONCAT __arg ${__arg} "--wrap=sys_tick,")
  string(CONCAT __arg ${__arg} "--wrap=sys_unix")
  list(APPEND arg "${${dst}}")
  list(APPEND arg "${__arg}")
  set(${dst} "${arg}" PARENT_SCOPE)
endfunction()
