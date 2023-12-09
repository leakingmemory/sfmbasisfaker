
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

add_executable(parseParameters assert.cpp parseParameters.cpp)
target_include_directories(parseParameters PRIVATE ../include)
target_link_libraries(parseParameters PRIVATE libfhir)
target_link_libraries(parseParameters PRIVATE cpprestsdk::cpprest -lcrypto)

add_test(NAME ParseGetmedReq COMMAND parseParameters "${CMAKE_SOURCE_DIR}/tests/data/nhnfhir/requests/getmed.json" request)
add_test(NAME ParseGetmedResp COMMAND parseParameters "${CMAKE_SOURCE_DIR}/tests/data/nhnfhir/magistral/magistral-getmed-response.json" response)
add_test(NAME ParseSendmedReq COMMAND parseParameters "${CMAKE_SOURCE_DIR}/tests/data/nhnfhir/requests/sendmed.json" sendmedrequest)
add_test(NAME ParseSendmedResp COMMAND parseParameters "${CMAKE_SOURCE_DIR}/tests/data/nhnfhir/responses/sendmed.json" sendmedresponse)
