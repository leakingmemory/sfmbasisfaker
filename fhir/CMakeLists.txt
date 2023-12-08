add_library(libfhir OBJECT fhir.cpp value.cpp medication.cpp fhirextendable.cpp extension.cpp fhirobject.cpp dosage.cpp
medstatement.cpp)

target_include_directories(libfhir PRIVATE "../include")
