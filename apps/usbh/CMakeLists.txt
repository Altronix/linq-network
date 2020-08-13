add_executable(usbh main.c)
target_link_libraries(usbh usbh-static common-static)
install(TARGETS usbh RUNTIME DESTINATION bin)
