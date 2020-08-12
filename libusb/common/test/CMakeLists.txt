append_mock_file_linker_flags(wrap_flags)
set(LIBS cmocka mock-file linq-usb-common-test-static utils-static)

add_executable(test-linq-usb-common main.c)
set_target_properties(test-linq-usb-common PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-linq-usb-common ${LIBS})
add_test(NAME test-linq-usb-common COMMAND test-linq-usb-common)
install(TARGETS test-linq-usb-common DESTINATION bin)
