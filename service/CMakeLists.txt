
add_library(service OBJECT DataDirectory.cpp DataDirectory.h PersonStorage.cpp PersonStorage.h
        CreatePrescriptionService.cpp CreatePrescriptionService.h PrescriptionStorage.cpp PrescriptionStorage.h)

target_include_directories(service PRIVATE ../include)
