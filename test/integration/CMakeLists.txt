# Alert integration Test
add_executable(integration-alert-test alerts.c)
target_link_libraries(integration-alert-test 
                      linq-common-fixture 
                      atx-net-static)
add_test(NAME integration-alert-test COMMAND integration-alert-test)

# Broadcast integration test
add_executable(integration-broadcast-test broadcast.c)
target_link_libraries(integration-broadcast-test 
                      linq-common-fixture 
                      atx-net-static)
add_test(NAME integration-broadcast-test COMMAND integration-broadcast-test)

# Response integration test
add_executable(integration-response-test response.c)
target_link_libraries(integration-response-test 
                      linq-common-fixture 
                      atx-net-static)
add_test(NAME integration-response-test COMMAND integration-response-test)

# Forward response integration test
add_executable(integration-forward-response-test forward_response.c)
target_link_libraries(integration-forward-response-test
                      linq-common-fixture
                      atx-net-static)
add_test(NAME integration-forward-response-test
         COMMAND integration-forward-response-test)

# 504 integration test
add_executable(integration-504-test process_504.c)
target_link_libraries(integration-504-test 
                      linq-common-fixture 
                      atx-net-static)
add_test(NAME integration-504-test COMMAND integration-504-test)
