add_executable(test-linq-cpp-binding main.cpp)
target_link_libraries(test-linq-cpp-binding linq-cpp-binding linq-c-test mock-czmq zmq czmq helpers cmocka)
set_target_properties(test-linq-cpp-binding PROPERTIES LINKER_LANGUAGE CXX)
add_test(NAME test-linq-cpp-binding COMMAND test-linq-cpp-binding)
