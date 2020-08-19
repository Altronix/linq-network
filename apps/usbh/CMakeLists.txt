add_executable(usbh main.c)
target_link_libraries(usbh network-static)
install(TARGETS usbh RUNTIME DESTINATION bin)
