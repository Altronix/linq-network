# --------------------------------------------------------------------------
# Lib of mock functions for test
# --------------------------------------------------------------------------
add_library(mock-mongoose 
  mock_mongoose.c
  parse_http.c
  parse_http.h)

target_link_libraries(mock-mongoose
  http-test
  mock-utils
  http-parser
  stdc++)

target_include_directories(mock-mongoose PUBLIC .)

function(mock_mongoose_linker_flags dst)
  string(CONCAT __arg "-Wl,")
  string(CONCAT __arg ${__arg} "--wrap=mg_mgr_init,")
  string(CONCAT __arg ${__arg} "--wrap=mg_mgr_free,")
  string(CONCAT __arg ${__arg} "--wrap=mg_set_protocol_http_websocket,")
  string(CONCAT __arg ${__arg} "--wrap=mg_mgr_poll,")
  string(CONCAT __arg ${__arg} "--wrap=mg_printf,")
  string(CONCAT __arg ${__arg} "--wrap=mg_send_websocket_frame,")
  string(CONCAT __arg ${__arg} "--wrap=mg_vprintf,")
  string(CONCAT __arg ${__arg} "--wrap=mg_printf_http_chunk,")
  string(CONCAT __arg ${__arg} "--wrap=mg_printf_html_escape,")
  string(CONCAT __arg ${__arg} "--wrap=mg_bind")
  list(APPEND arg "${${dst}}")
  list(APPEND arg "${__arg}")
  set(${dst} "${arg}" PARENT_SCOPE)
endfunction()

mock_mongoose_linker_flags(mongoose_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${mongoose_flags} ${utils_flags}")

add_executable(test-mock-mongoose main.c)
set_target_properties(test-mock-mongoose PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-mock-mongoose
                      mock-mongoose
                      cmocka
                      helpers)
add_test(NAME test-mock-mongoose COMMAND test-mock-mongoose)
install(TARGETS test-mock-mongoose DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

