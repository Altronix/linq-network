set(HTTP_PARSER_SOURCE_DIR ${EXTERNAL_DIR}/http_parser)
set(HTTP_PARSER_TEST_FILE ${HTTP_PARSER_SOURCE_DIR}/http_parser.c)
check_extract("${DOWNLOAD_DIR}/http_parser.tar.gz" "${HTTP_PARSER_TEST_FILE}")

add_library(http-parser STATIC
	${HTTP_PARSER_SOURCE_DIR}/http_parser.c 
	${HTTP_PARSER_SOURCE_DIR}/http_parser.h)
target_include_directories(http-parser PUBLIC ${HTTP_PARSER_SOURCE_DIR})

install(TARGETS http-parser DESTINATION lib)
