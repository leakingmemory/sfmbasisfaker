add_library(libfhir OBJECT fhir.cpp value.cpp medication.cpp fhirextendable.cpp extension.cpp fhirobject.cpp dosage.cpp)

target_include_directories(libfhir PRIVATE "../include")
