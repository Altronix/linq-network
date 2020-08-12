append_mock_file_linker_flags(wrap_flags)
append_mock_libusb_linker_flags(wrap_flags)
set(LIBS cmocka mock-file mock-libusb linq-usbh-test-static linq-utils-static)

add_executable(test-linq-usbh-device main.c)
set_target_properties(test-linq-usbh-device PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-linq-usbh-device ${LIBS})
add_test(NAME test-linq-usbh-device COMMAND test-linq-usbh-device)
install(TARGETS test-linq-usbh-device DESTINATION bin)

