# --------------------------------------------------------------------------
# Lib of mock functions for test
# --------------------------------------------------------------------------
add_library(mock-sqlite sqlite3.c sqlite3.h mock_sqlite.c)

target_link_libraries(mock-sqlite
  network-test-static
  mock-utils
  stdc++)

target_include_directories(mock-sqlite PUBLIC .)

function(mock_sqlite_linker_flags dst)
  string(CONCAT __arg "-Wl,")
  string(CONCAT __arg ${__arg} "--wrap=sqlite3_open_v2,")
  string(CONCAT __arg ${__arg} "--wrap=sqlite3_close,")
  string(CONCAT __arg ${__arg} "--wrap=sqlite3_prepare_v2,")
  string(CONCAT __arg ${__arg} "--wrap=sqlite3_prepare_v3,")
  string(CONCAT __arg ${__arg} "--wrap=sqlite3_step,")
  string(CONCAT __arg ${__arg} "--wrap=sqlite3_column_text,")
  string(CONCAT __arg ${__arg} "--wrap=sqlite3_column_int,")
  string(CONCAT __arg ${__arg} "--wrap=sqlite3_finalize,")
  string(CONCAT __arg ${__arg} "--wrap=sqlite3_exec")
  list(APPEND arg "${${dst}}")
  list(APPEND arg "${__arg}")
  set(${dst} "${arg}" PARENT_SCOPE)
endfunction()

append_sqlite_compiler_flags(definitions)

mock_sqlite_linker_flags(sqlite_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${sqlite_flags} ${utils_flags}")

add_executable(test-mock-sqlite main.c)
set_target_properties(test-mock-sqlite PROPERTIES LINK_FLAGS ${wrap_flags})
target_compile_definitions(test-mock-sqlite PRIVATE ${definitions})
target_link_libraries(test-mock-sqlite
                      mock-sqlite
                      cmocka
                      helpers)
add_test(NAME test-mock-sqlite COMMAND test-mock-sqlite)
install(TARGETS test-mock-sqlite DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


