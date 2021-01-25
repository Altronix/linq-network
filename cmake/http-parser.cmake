### Build http_parser ###
set(HTTP_PARSER_SOURCE_DIR ${EXTERNAL_DIR}/http_parser)

if(NOT EXISTS ${HTTP_PARSER_SOURCE_DIR}/http_parser.c)
	execute_process(
		COMMAND ${GIT_EXECUTABLE} submodule update --init ${HTTP_PARSER_SOURCE_DIR}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		RESULT_VARIABLE GIT_CLONE_HTTP_PARSER_RESULT)
	message(STATUS "GIT_CLONE_HTTP_PARSER_RESULT: ${GIT_CLONE_HTTP_PARSER_RESULT}")
endif()


add_library(http-parser 
	${HTTP_PARSER_SOURCE_DIR}/http_parser.c 
	${HTTP_PARSER_SOURCE_DIR}/http_parser.h)
target_include_directories(http-parser PUBLIC ${HTTP_PARSER_SOURCE_DIR})
