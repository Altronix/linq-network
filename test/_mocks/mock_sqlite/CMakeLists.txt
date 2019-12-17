# --------------------------------------------------------------------------
# Lib of mock functions for test
# --------------------------------------------------------------------------
add_library(mock-sqlite mock_sqlite.c)

target_link_libraries(mock-sqlite
  linq-netw-test
  cmocka
  stdc++)

target_include_directories(mock-sqlite PUBLIC .)

function(mock_sqlite_linker_flags dst)
  string(CONCAT wrap_flags "-Wl,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_mgr_init,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_mgr_free,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_set_protocol_http_websocket,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_mgr_poll,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_printf,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_vprintf,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_printf_http_chunk,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_printf_html_escape,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_bind")
  set(${dst} ${wrap_flags} PARENT_SCOPE)
endfunction()

mock_sqlite_linker_flags(wrap_flags)

add_executable(test-mock-sqlite main.c)
set_target_properties(test-mock-sqlite PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-mock-sqlite
                      mock-sqlite
                      cmocka
                      helpers)
add_test(NAME test-mock-sqlite COMMAND test-mock-sqlite)
install(TARGETS test-mock-sqlite DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


