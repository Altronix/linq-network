set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(linq-cpp-binding linq.hpp)
set_target_properties(linq-cpp-binding PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(linq-cpp-binding linq-c-test mock-czmq zmq czmq helpers jsmn klib ${CMAKE_THREAD_LIBS_INIT} rt m stdc++ uuid)
target_include_directories(linq-cpp-binding PUBLIC ./)
