set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(linq-netw-cpp-binding INTERFACE)
target_include_directories(linq-netw-cpp-binding INTERFACE ./)
set_target_properties(linq-netw-cpp-binding PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR})
install(TARGETS linq-netw-cpp-binding
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include/altronix)
