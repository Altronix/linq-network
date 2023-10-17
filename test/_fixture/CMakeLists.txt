add_library(linq-common-fixture fixture.c fixture.h)
target_link_libraries(linq-common-fixture
                      http-parser
                      wolfssl
                      crypt
		      rt
		      m)
target_include_directories(linq-common-fixture PUBLIC ./)
