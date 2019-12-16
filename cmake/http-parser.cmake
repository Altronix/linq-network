### Build http_parser ###
set(http_parser_SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/http_parser)
add_library(http-parser 
	${http_parser_SOURCE_DIR}/http_parser.c 
	${http_parser_SOURCE_DIR}/http_parser.h)
target_include_directories(http-parser PUBLIC ${http_parser_SOURCE_DIR})
