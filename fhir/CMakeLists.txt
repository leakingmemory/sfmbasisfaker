add_library(libfhir OBJECT fhir.cpp value.cpp medication.cpp fhirextendable.cpp extension.cpp fhirobject.cpp dosage.cpp
        medstatement.cpp bundleentry.cpp fhirparser.cpp bundle.cpp compositionsection.cpp composition.cpp person.cpp
        organization.cpp substance.cpp)

target_include_directories(libfhir PRIVATE "../include")
