add_library(linq-common-fixture fixture.c fixture.h)
target_link_libraries(linq-common-fixture
                      helpers
                      wolfssl
                      http-parser
                      crypt)
target_include_directories(linq-common-fixture PUBLIC ./)
