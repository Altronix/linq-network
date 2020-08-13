append_mock_file_linker_flags(wrap_flags)
set(LIBS cmocka mock-file usbd-test-static common-static)

add_executable(test-usbd main.c)
set_target_properties(test-usbd PROPERTIES LINK_FLAGS ${wrap_flags})
target_link_libraries(test-usbd ${LIBS})
add_test(NAME test-usbd COMMAND test-usbd)
install(TARGETS test-usbd DESTINATION bin)
