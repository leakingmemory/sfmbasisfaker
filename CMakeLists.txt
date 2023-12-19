cmake_minimum_required(VERSION 3.27)
project(sfmbasisfaker)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(cpprestsdk REQUIRED)

add_subdirectory(webserver)
add_subdirectory(controllers)

add_executable(sfmbasisfaker main.cpp)

target_link_libraries(sfmbasisfaker PRIVATE webserver)
target_link_libraries(sfmbasisfaker PRIVATE controllers)
target_link_libraries(sfmbasisfaker PRIVATE -lsfmbasisapi)
target_link_libraries(sfmbasisfaker PRIVATE cpprestsdk::cpprest -lcrypto)

install(TARGETS sfmbasisfaker DESTINATION bin)
