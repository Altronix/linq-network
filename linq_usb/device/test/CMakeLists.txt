append_mock_file_linker_flags(wrap_flags)
set(LIBS cmocka mock-file linq-usb-test-static linq-utils-static)

add_executable(test-linq-usb-device main.c)
set_target_properties(test-linq-usb-device PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-linq-usb-device ${LIBS})
add_test(NAME test-linq-usb-device COMMAND test-linq-usb-device)
install(TARGETS test-linq-usb-device DESTINATION bin)
