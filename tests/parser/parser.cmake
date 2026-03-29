add_executable(test_parser
    ${CMAKE_CURRENT_LIST_DIR}/test_parser.c
    ${SRC_ROOT}/src/spectral_parser.c
    ${CMAKE_SOURCE_DIR}/stubs/gpio_stub.c
)

target_include_directories(test_parser PRIVATE
    ${CMAKE_SOURCE_DIR}/stubs
    ${SRC_ROOT}/inc
    ${UNITY_DIR}/src
)

target_link_libraries(test_parser
    unity
    stubs
)

add_test(NAME parser COMMAND test_parser)