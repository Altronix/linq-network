add_executable(usbh main.c)
target_link_libraries(usbh linq-usbh-static linq-utils-static)
install(TARGETS usbh RUNTIME DESTINATION bin)
