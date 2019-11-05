add_executable(integration-alert-test alerts.c)
target_link_libraries(integration-alert-test linq linq-common-fixture)
add_test(NAME integration-alert-test COMMAND integration-alert-test)

add_executable(integration-broadcast-test broadcast.c)
target_link_libraries(integration-broadcast-test linq linq-common-fixture)
add_test(NAME integration-broadcast-test COMMAND integration-broadcast-test)

add_executable(integration-response-test response.c)
target_link_libraries(integration-response-test linq linq-common-fixture)
add_test(NAME integration-response-test COMMAND integration-response-test)

add_executable(integration-forward-response-test forward_response.c)
target_link_libraries(integration-forward-response-test linq
                      linq-common-fixture)
add_test(NAME integration-forward-response-test
         COMMAND integration-forward-response-test)
