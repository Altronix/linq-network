add_executable(usbh main.c)
target_link_libraries(usbh usbh-static utils-static)
install(TARGETS usbh RUNTIME DESTINATION bin)
