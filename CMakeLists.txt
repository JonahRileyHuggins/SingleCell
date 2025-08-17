cmake_minimum_required(VERSION 3.15...4.0)
project(SingleCell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# === Paths ===

set(BLA_VENDOR OpenBLAS)
find_package(BLAS REQUIRED)

# === Git External Projects === 
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/libxml2/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# --- Build libxml2 as external project ---
# include(ExternalProject)
# ExternalProject_Add(xml2
#     SOURCE_DIR ${CMAKE_SOURCE_DIR}/extern/libxml2
#     BINARY_DIR ${CMAKE_BINARY_DIR}/extern/libxml2-build
#     INSTALL_DIR ${CMAKE_BINARY_DIR}/extern/libxml2-install
#     CONFIGURE_COMMAND ${CMAKE_COMMAND} -S <SOURCE_DIR> -B <BINARY_DIR>
#         -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
#     BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target all -- -j
#     INSTALL_COMMAND ${CMAKE_COMMAND} --install <BINARY_DIR>
# )

# # Tell libsbml where to find libxml2
# set(LIBXML_INCLUDE_DIR
#     ${CMAKE_BINARY_DIR}/extern/libxml2-install/include/libxml2
#     CACHE PATH "libxml2 include dir"
# )
# set(LIBXML_LIBRARY
#     ${CMAKE_BINARY_DIR}/extern/libxml2-install/lib/libxml2.a
#     CACHE FILEPATH "libxml2 library"
# )

# --- Run AMICI build scripts at configure time (simple approach) ---
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/extern")
set(AMICI_DIR "${THIRD_PARTY_DIR}/AMICI")
set(AMICI_SCRIPTS_DIR "${AMICI_DIR}/scripts")
set(AMICI_INCLUDE_DIR "${AMICI_DIR}/include")
set(AMICI_LIB "${AMICI_DIR}/build/libamici.a")
set(AMICI_STAMP "${CMAKE_BINARY_DIR}/_extern_amici_built.stamp")

if(NOT EXISTS "${AMICI_LIB}")
    message(STATUS "AMICI library not found at ${AMICI_DIR}. Running AMICI build scripts (configure-time).")

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

# === External dependencies ===
add_subdirectory(extern/pybind11)
add_subdirectory(extern/libxml2)
add_subdirectory(extern/muparser)
add_subdirectory(extern/libsbml)
add_dependencies(sbml xml2)

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

# === Internal Dependencies ===  
add_subdirectory(amici_models/deterministic)
add_subdirectory(amici_models/One4All)

# === Executable target ===
add_executable(${PROJECT_NAME} ${SINGLECELL_SRC_LIST})

target_include_directories(SingleCell PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${AMICI_INCLUDE_DIR}
)

target_link_libraries(SingleCell PRIVATE
    ${AMICI_LIB}
    deterministic
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
    deterministic
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

# Update the environment PYSINGLECELL_PATH variable:
add_custom_command(TARGET pySingleCell POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "$ENV{HOME}/.config/singlecell"
  COMMAND /bin/sh -c "printf '%s\n' '${CMAKE_BINARY_DIR}' > \"$ENV{HOME}/.config/singlecell/pysinglecell_path\""
  COMMENT "Recording pySingleCell build dir to \$HOME/.config/singlecell/pysinglecell_path"
)