
add_library(serialization OBJECT code.cpp prescription.cpp pll.cpp pllfhir.cpp)

target_include_directories(serialization PRIVATE ../include)
