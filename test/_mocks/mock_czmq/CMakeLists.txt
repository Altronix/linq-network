# --------------------------------------------------------------------------
# Lib of mock functions for test
# --------------------------------------------------------------------------
add_library(mock-czmq mock_zmsg.c mock_zsock.c mock_zpoll.c)
target_link_libraries(mock-czmq
                      linq-network-test-static
                      mock-utils
                      ${LIBZMQ_STATIC_LIBS}
		      stdc++
		      uuid)
target_include_directories(mock-czmq PUBLIC .)

function(mock_czmq_linker_flags dst)
  string(CONCAT wrap "-Wl,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_poll,")
  string(CONCAT wrap ${wrap} "--wrap=zmsg_recv,")
  string(CONCAT wrap ${wrap} "--wrap=zmsg_send,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_new_router,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_new_dealer,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_type_str,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_destroy,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_new_router_checked,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_new_dealer_checked,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_destroy_checked,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_bind,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_resolve,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_connect,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_send,")
  string(CONCAT wrap ${wrap} "--wrap=zsock_recv")
  set(${dst} ${wrap} PARENT_SCOPE)
endfunction()

mock_czmq_linker_flags(czmq_flags)
mock_utils_linker_flags(utils_flags)
set(wrap_flags "${czmq_flags} ${utils_flags}")

add_executable(test-mock-czmq main.c)
set_target_properties(test-mock-czmq PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-mock-czmq
                      mock-czmq
                      cmocka
                      helpers
                      linq-network-test-static
                      mock-utils
		      mock-czmq)
add_test(NAME test-mock-czmq COMMAND test-mock-czmq)
install(TARGETS test-mock-czmq DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
