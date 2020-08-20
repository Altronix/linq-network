add_executable(atx-ttyd 
  main.c 
  make_request.c
  make_request.h
  http_parser.c 
  http_parser.h)
target_link_libraries(atx-ttyd usbd-static common-static)
install(TARGETS atx-ttyd RUNTIME DESTINATION bin)
