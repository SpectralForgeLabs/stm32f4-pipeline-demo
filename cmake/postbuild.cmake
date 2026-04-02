message(STATUS "Postbuild: artifact dir = ${ARTIFACT_DIR}")
message(STATUS "Postbuild: stamped basename = ${STAMPED_BASENAME}")

foreach(ext bin hex elf)
    set(IN_FILE  "${ARTIFACT_DIR}/${PROJECT_NAME}.${ext}")
    if(EXISTS "${IN_FILE}")
        set(OUT_FILE "${ARTIFACT_DIR}/${STAMPED_BASENAME}.${ext}")

        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${IN_FILE}" "${OUT_FILE}"
            COMMAND_ERROR_IS_FATAL ANY
        )

        message(STATUS "Postbuild: created ${OUT_FILE}")
    else()
        message(STATUS "Postbuild: skipping missing ${IN_FILE}")
    endif()
endforeach()

set(MANIFEST_FILE "${ARTIFACT_DIR}/${STAMPED_BASENAME}.txt")
file(WRITE "${MANIFEST_FILE}"
"branch=${GIT_BRANCH}\nsha=${GIT_SHA}\nshort_sha=${GIT_SHA_SHORT}\nbuild=${BUILD_NUMBER}\n")
message(STATUS "Postbuild: wrote ${MANIFEST_FILE}")

set(TAR_FILE "${ARTIFACT_DIR}/${STAMPED_BASENAME}.tar")
execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar cfv "${TAR_FILE}"
            "${STAMPED_BASENAME}.bin"
            "${STAMPED_BASENAME}.hex"
            "${STAMPED_BASENAME}.elf"
            "${STAMPED_BASENAME}.txt"
    WORKING_DIRECTORY "${ARTIFACT_DIR}"
    COMMAND_ERROR_IS_FATAL ANY
)

message(STATUS "Postbuild: created ${TAR_FILE}")