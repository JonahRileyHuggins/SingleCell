cmake_minimum_required(VERSION 3.10)
project(SingleCell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# === Paths ===

set(BLA_VENDOR OpenBLAS)
find_package(BLAS REQUIRED)

# --- Run AMICI build scripts at configure time (simple approach) ---
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/ThirdParty")
set(AMICI_DIR "${THIRD_PARTY_DIR}/AMICI")
set(AMICI_SCRIPTS_DIR "${AMICI_DIR}/scripts")
set(AMICI_INCLUDE_DIR "${AMICI_DIR}/include")
set(AMICI_LIB "${AMICI_DIR}/build/libamici.a")
set(AMICI_STAMP "${CMAKE_BINARY_DIR}/_thirdparty_amici_built.stamp")

if(NOT EXISTS "${AMICI_LIB}")
    message(STATUS "AMICI library not found at ${AMICI_LIB_PATH}. Running AMICI build scripts (configure-time).")

    # Run SuiteSparse install:
    execute_process(
        COMMAND bash "${AMICI_SCRIPTS_DIR}/buildSuiteSparse.sh"
        WORKING_DIRECTORY "${AMICI_SCRIPTS_DIR}"
        RESULT_VARIABLE _res_suitesparse
        OUTPUT_VARIABLE _out_suitesparse
        ERROR_VARIABLE _err_suitesparse
    )

    if(NOT _res_suitesparse EQUAL 0)
        message(FATAL_ERROR "buildSuiteSparse.sh failed (exit ${_res_suitesparse}).\nstdout:\n${_out_suitesparse}\nstderr:\n${_err_suitesparse}")
    endif()

    # Run Sundials install:
    execute_process(
        COMMAND bash "${AMICI_SCRIPTS_DIR}/buildSundials.sh"
        WORKING_DIRECTORY "${AMICI_SCRIPTS_DIR}"
        RESULT_VARIABLE _res_sundials
        OUTPUT_VARIABLE _out_sundials
        ERROR_VARIABLE _err_sundials
    )

    if(NOT _res_sundials EQUAL 0)
        message(FATAL_ERROR "buildSundials.sh failed (exit ${_res_sundials}).\nstdout:\n${_out_sundials}\nstderr:\n${_err_sundials}")
    endif()

    # Run buildAmici script
    execute_process(
      COMMAND bash "${AMICI_SCRIPTS_DIR}/buildAmici.sh"
      WORKING_DIRECTORY "${AMICI_SCRIPTS_DIR}"
      RESULT_VARIABLE _res_amici
      OUTPUT_VARIABLE _out_amici
      ERROR_VARIABLE _err_amici
    )
    if(NOT _res_amici EQUAL 0)
      message(FATAL_ERROR "buildAmici.sh failed (exit ${_res_amici}).\nstdout:\n${_out_amici}\nstderr:\n${_err_amici}")
    endif()

    # Create a small stamp file so configure won't run scripts again unless the artifact is removed.
    file(WRITE "${AMICI_STAMP}" "AMICI built by CMake configure at: ${CMAKE_CURRENT_LIST_DIR}\n")
    message(STATUS "AMICI build scripts finished; expected library: ${AMICI_LIB}")
else()
    message(STATUS "AMICI library already present at ${AMICI_LIB}; skipping configure-time build.")
endif()

# Expose the Amici_DIR so find_package can find the config produced by AMICI install
set(Amici_DIR "${AMICI_DIR}/build" CACHE PATH "AMICI package config dir")
list(APPEND CMAKE_PREFIX_PATH "${Amici_DIR}")

# === Source Files ===
set(SINGLECELL_SRC_LIST
    src/SBMLHandler.cpp
    src/BaseModule.cpp
    src/DeterministicModule.cpp
    src/StochasticModule.cpp
    src/One4AllModule.cpp
    src/SingleCell.cpp
    src/utils.cpp
    src/ArgParsing.cpp
    src/main.cpp)

# === External dependencies ===
add_subdirectory(amici_models/Deterministic)
add_subdirectory(amici_models/One4All)
add_subdirectory(ThirdParty/pybind11)
add_subdirectory(ThirdParty/libxml2-2.14.3)
add_subdirectory(ThirdParty/muparser)
add_subdirectory(ThirdParty/libsbml-5.20.2)

# === Executable target ===
add_executable(${PROJECT_NAME} ${SINGLECELL_SRC_LIST})

target_include_directories(SingleCell PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${AMICI_INCLUDE_DIR}
)

target_link_libraries(SingleCell PRIVATE
    ${AMICI_LIB}
    Deterministic
    One4All
    xml2
    sbml
    muparser
    pybind11::module
)


# Copy executable to python directory after build
add_custom_command(TARGET SingleCell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:SingleCell>
        ${CMAKE_SOURCE_DIR}/src/
)

# === Pybind11 module ===
pybind11_add_module(pySingleCell src/bindings.cpp ${SINGLECELL_SRC_LIST})

target_include_directories(pySingleCell PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${AMICI_INCLUDE_DIR}
)

target_link_libraries(pySingleCell PRIVATE
    ${AMICI_LIB}
    Deterministic
    One4All
    xml2
    sbml
    muparser
    pybind11::module
)



# Output directory for .so / .pyd
set_target_properties(pySingleCell PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
