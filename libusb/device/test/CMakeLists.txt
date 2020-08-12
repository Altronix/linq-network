append_mock_file_linker_flags(wrap_flags)
set(LIBS cmocka mock-file usbd-test-static utils-static)

add_executable(test-usbd-device main.c)
set_target_properties(test-usbd-device PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-usbd-device ${LIBS})
add_test(NAME test-usbd-device COMMAND test-usbd-device)
install(TARGETS test-usbd-device DESTINATION bin)
