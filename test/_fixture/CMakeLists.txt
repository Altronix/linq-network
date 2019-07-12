add_executable(linq-common-fixture main.c)
target_link_libraries(linq-common-fixture wolfssl http-parser crypt)
