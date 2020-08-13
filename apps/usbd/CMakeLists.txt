add_executable(usbd main.c)
target_link_libraries(usbd usbd-static common-static)
install(TARGETS usbd RUNTIME DESTINATION bin)
