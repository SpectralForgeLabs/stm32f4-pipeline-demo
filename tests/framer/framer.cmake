add_executable(test_framer
    ${CMAKE_CURRENT_LIST_DIR}/test_framer.c
    ${SRC_ROOT}/src/spectral_framer.c
)

target_include_directories(test_framer PRIVATE
    ${SRC_ROOT}/inc
    ${UNITY_DIR}/src
)

target_link_libraries(test_framer
    unity
)

add_test(NAME framer COMMAND test_framer)