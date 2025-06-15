cmake_minimum_required(VERSION 3.27)
project(sfmbasisfaker)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    add_compile_options("-I/usr/local/include")
    add_compile_options("-DFREEBSD")
    add_link_options("-L/usr/local/lib")
endif()

find_package(cpprestsdk REQUIRED)

add_subdirectory(webserver)
add_subdirectory(controllers)
add_subdirectory(service)
add_subdirectory(serialization)

add_executable(sfmbasisfaker main.cpp)

target_link_libraries(sfmbasisfaker PRIVATE webserver)
target_link_libraries(sfmbasisfaker PRIVATE controllers)
target_link_libraries(sfmbasisfaker PRIVATE service)
target_link_libraries(sfmbasisfaker PRIVATE serialization)
target_link_libraries(sfmbasisfaker PRIVATE -lsfmbasisapi -ljjwtid)
target_link_libraries(sfmbasisfaker PRIVATE cpprestsdk::cpprest -lcrypto)

install(TARGETS sfmbasisfaker DESTINATION bin)
