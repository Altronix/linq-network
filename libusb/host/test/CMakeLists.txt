append_mock_file_linker_flags(wrap_flags)
append_mock_libusb_linker_flags(wrap_flags)
set(LIBS cmocka mock-file mock-libusb usbh-test-static utils-static)

add_executable(test-usbh-device main.c)
set_target_properties(test-usbh-device PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-usbh-device ${LIBS})
add_test(NAME test-usbh-device COMMAND test-usbh-device)
install(TARGETS test-usbh-device DESTINATION bin)

