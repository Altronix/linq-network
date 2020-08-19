add_executable(atx-usbd main.c)
target_link_libraries(atx-usbd usbd-static common-static)
install(TARGETS atx-usbd RUNTIME DESTINATION bin)
