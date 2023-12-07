add_library(libfhir OBJECT fhir.cpp  value.cpp)

target_include_directories(libfhir PRIVATE "../include")
