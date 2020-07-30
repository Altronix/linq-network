add_executable(usbd main.c)
target_link_libraries(usbd linq-usbd-static)
install(TARGETS usbd RUNTIME DESTINATION bin)
