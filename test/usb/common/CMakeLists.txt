append_mock_file_linker_flags(wrap_flags)
set(LIBS cmocka mock-file network-test-static)

add_executable(test-usb-common main.c)
set_target_properties(test-usb-common PROPERTIES LINK_FLAGS "${wrap_flags}")
target_link_libraries(test-usb-common ${LIBS})
add_test(NAME test-usb-common COMMAND test-usb-common)
install(TARGETS test-usb-common DESTINATION bin)
