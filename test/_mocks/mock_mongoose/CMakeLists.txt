# --------------------------------------------------------------------------
# Lib of mock functions for test
# --------------------------------------------------------------------------
add_library(mock-mongoose mock_mongoose.c)
target_link_libraries(mock-mongoose
                      linq-netw-test
		      stdc++)
target_include_directories(mock-mongoose PUBLIC .)

function(mock_mongoose_linker_flags dst)
  string(CONCAT wrap_flags "-Wl,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_mgr_init,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_mgr_free,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_mgr_poll,")
  string(CONCAT wrap_flags ${wrap_flags} "--wrap=mg_mgr_bind")
  set(${dst} ${wrap_flags} PARENT_SCOPE)
endfunction()

mock_mongoose_linker_flags(wrap_flags)

add_executable(test-mock-mongoose main.c)
set_target_properties(test-mock-mongoose PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-mock-mongoose
                      mock-mongoose
                      cmocka
                      helpers)
add_test(NAME test-mock-mongoose COMMAND test-mock-mongoose)
install(TARGETS test-mock-mongoose DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

