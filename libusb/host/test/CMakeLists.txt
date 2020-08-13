append_mock_file_linker_flags(wrap_flags)
append_mock_libusb_linker_flags(wrap_flags)
set(LIBS cmocka mock-file mock-libusb usbh-test-static common-static)

add_executable(test-usbh main.c)
set_target_properties(test-usbh PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-usbh ${LIBS})
add_test(NAME test-usbh COMMAND test-usbh)
install(TARGETS test-usbh DESTINATION bin)

