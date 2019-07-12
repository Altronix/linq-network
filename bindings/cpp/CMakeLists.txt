add_library(linq-cpp-binding linq.hpp)
set_target_properties(linq-cpp-binding PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(linq-cpp-binding linq-unix czmq zmq jsmn klib ${CMAKE_THREAD_LIBS_INIT} rt m stdc++ uuid)
target_include_directories(linq-cpp-binding PUBLIC ./)
