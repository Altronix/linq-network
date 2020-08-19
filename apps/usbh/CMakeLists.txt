add_executable(atx-usbh main.c)
target_link_libraries(atx-usbh network-static)
install(TARGETS atx-usbh RUNTIME DESTINATION bin)
