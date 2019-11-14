set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(linq-io-cpp-binding INTERFACE)
target_include_directories(linq-io-cpp-binding INTERFACE ./)
install(TARGETS linq-io-cpp-binding
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        INCLUDES DESTINATION include/altronix)
