
add_executable(valuetest valuetest.cpp assert.cpp)
target_include_directories(valuetest PRIVATE ../include)
target_link_libraries(valuetest PRIVATE libfhir)
target_link_libraries(valuetest PRIVATE cpprestsdk::cpprest -lcrypto)

add_test(NAME ValueTest COMMAND valuetest)

add_executable(parseMedication assert.cpp parseMedication.cpp)
target_include_directories(parseMedication PRIVATE ../include)
target_link_libraries(parseMedication PRIVATE libfhir)
target_link_libraries(parseMedication PRIVATE cpprestsdk::cpprest -lcrypto)

add_test(NAME ParseMedicationMagistral COMMAND parseMedication "${CMAKE_SOURCE_DIR}/tests/data/nhnfhir/magistral/magistral-medication.json" magistral)

add_executable(parseMedicationStatement assert.cpp parseMedicationStatement.cpp)
target_include_directories(parseMedicationStatement PRIVATE ../include)
target_link_libraries(parseMedicationStatement PRIVATE libfhir)
target_link_libraries(parseMedicationStatement PRIVATE cpprestsdk::cpprest -lcrypto)

add_test(NAME ParseMedicationStatementMagistral COMMAND parseMedicationStatement "${CMAKE_SOURCE_DIR}/tests/data/nhnfhir/magistral/magistral-medication-statement.json" magistral)

add_executable(parseBundle assert.cpp parseBundle.cpp)
target_include_directories(parseBundle PRIVATE ../include)
target_link_libraries(parseBundle PRIVATE libfhir)
target_link_libraries(parseBundle PRIVATE cpprestsdk::cpprest -lcrypto)

add_test(NAME ParseBundleMagistral COMMAND parseBundle "${CMAKE_SOURCE_DIR}/tests/data/nhnfhir/magistral/magistral-bundle.json" magistral)
