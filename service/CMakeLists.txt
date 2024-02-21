
add_library(service OBJECT DataDirectory.cpp DataDirectory.h PersonStorage.cpp PersonStorage.h)

target_include_directories(service PRIVATE ../include)
