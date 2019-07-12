add_executable(alert-integration-test alerts.c)
target_link_libraries(alert-integration-test linq-c linq-common-fixture)
add_test(NAME alert-integration-test COMMAND alert-integration-test)
