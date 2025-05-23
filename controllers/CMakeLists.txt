
add_library(controllers OBJECT MedicationController.cpp
        PharmacyController.cpp
        PharmacyController.h)

target_include_directories(controllers PRIVATE ../include)
