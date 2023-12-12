cmake_minimum_required(VERSION 3.27)
project(sfmbasisfaker)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(cpprestsdk REQUIRED)

add_subdirectory(fhir)
add_subdirectory(webserver)
add_subdirectory(controllers)
add_subdirectory(nhnfhir)

add_executable(sfmbasisfaker main.cpp)

target_link_libraries(sfmbasisfaker PRIVATE libfhir)
target_include_directories(sfmbasisfaker PRIVATE include)

target_link_libraries(sfmbasisfaker PRIVATE webserver)
target_link_libraries(sfmbasisfaker PRIVATE controllers)
target_link_libraries(sfmbasisfaker PRIVATE nhnfhir)
target_link_libraries(sfmbasisfaker PRIVATE cpprestsdk::cpprest -lcrypto)

enable_testing()
add_subdirectory(tests)
