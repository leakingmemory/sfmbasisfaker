
add_executable(valuetest valuetest.cpp assert.cpp)
target_include_directories(valuetest PRIVATE ../include)
target_link_libraries(valuetest PRIVATE libfhir)
target_link_libraries(valuetest PRIVATE cpprestsdk::cpprest -lcrypto)

add_test(NAME ValueTest COMMAND valuetest)
