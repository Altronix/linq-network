add_library(mock-libusb mock_libusb.c mock_libusb.h)
target_link_libraries(mock-libusb cmocka stdc++ network-static libusb-static)
target_include_directories(mock-libusb PUBLIC .)

function (append_mock_libusb_linker_flags dst)
  string(CONCAT __arg "-Wl,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_init,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_exit,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_open,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_close,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_control_transfer,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_kernel_driver_active,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_detach_kernel_driver,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_attach_kernel_driver,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_claim_interface,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_set_option,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_get_device_list,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_get_string_descriptor_ascii,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_free_device_list,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_get_config_descriptor,")
  string(CONCAT __arg ${__arg} "--wrap=libusb_get_device_descriptor")
  list(APPEND arg "${${dst}}")
  list(APPEND arg "${__arg}")
  set(${dst} "${arg}" PARENT_SCOPE)
endfunction()
append_mock_libusb_linker_flags(wrap_flags)

add_executable(test-mock-libusb main.c)
set_target_properties(test-mock-libusb PROPERTIES LINK_FLAGS "${wrap_flags}")
target_link_libraries(test-mock-libusb mock-libusb cmocka)
add_test(NAME test-mock-libusb COMMAND test-mock-libusb)
install(TARGETS test-mock-libusb DESTINATION bin)
