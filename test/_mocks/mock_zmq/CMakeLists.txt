function(mock_zmq_linker_flags dst)
  string(CONCAT wrap "-Wl,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_ctx_new,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_ctx_destroy,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_socket,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_bind,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_connect,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_close,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_poll,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_msg_close,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_msg_more,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_msg_send,")
  string(CONCAT wrap ${wrap} "--wrap=zmq_msg_recv")
  list(APPEND arg "${${dst}}")
  list(APPEND arg "${wrap}")
  set(${dst} "${arg}" PARENT_SCOPE)
endfunction()
mock_zmq_linker_flags(wrap_flags)

add_library(mock-message message.cpp message.h)
target_include_directories(mock-message PRIVATE "${ZMQ_SOURCE_DIR}/src")
target_include_directories(mock-message PUBLIC ".")
target_link_libraries(mock-message ${ZMQ_LIBS})

add_library(mock-zmq mock_zmq.c mock_zmq.h)
set_target_properties(mock-zmq PROPERTIES LINK_FLAGS "${wrap_flags}")
target_link_libraries(mock-zmq 
  network-static
  mock-utils 
  ${ZMQ_LIBS} 
  mock-message
  ${CMAKE_THREAD_LIBS_INIT}
  stdc++ 
  uuid)
target_include_directories(mock-zmq PUBLIC .)

add_executable(test-mock-zmq main.c)
set_target_properties(test-mock-zmq PROPERTIES LINK_FLAGS "${wrap_flags}")
target_link_libraries(test-mock-zmq mock-zmq mock-utils stdc++ uuid)
add_test(NAME test-mock-zmq COMMAND test-mock-zmq)
install(TARGETS test-mock-zmq DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
