
add_library(service OBJECT DataDirectory.cpp DataDirectory.h PersonStorage.cpp PersonStorage.h
        CreatePrescriptionService.cpp CreatePrescriptionService.h PrescriptionStorage.cpp PrescriptionStorage.h
        PllStorage.cpp PllStorage.h DateTime.cpp DateTime.h)

target_include_directories(service PRIVATE ../include)
