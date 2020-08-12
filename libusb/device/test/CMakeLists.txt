append_mock_file_linker_flags(wrap_flags)
set(LIBS cmocka mock-file linq-usbd-test-static utils-static)

add_executable(test-linq-usbd-device main.c)
set_target_properties(test-linq-usbd-device PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-linq-usbd-device ${LIBS})
add_test(NAME test-linq-usbd-device COMMAND test-linq-usbd-device)
install(TARGETS test-linq-usbd-device DESTINATION bin)
