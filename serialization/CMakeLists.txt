
add_library(serialization OBJECT code.cpp prescription.cpp)

target_include_directories(serialization PRIVATE ../include)
