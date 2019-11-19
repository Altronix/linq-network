set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(linq-netw-cpp-binding INTERFACE)
target_include_directories(linq-netw-cpp-binding INTERFACE ./)
install(TARGETS linq-netw-cpp-binding
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include/altronix)
