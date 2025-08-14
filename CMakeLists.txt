cmake_minimum_required(VERSION 3.10)
project(SingleCell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# === Paths ===
set(THIRD_PARTY_DIR "${CMAKE_SOURCE_DIR}/ThirdParty")
set(Amici_DIR "${THIRD_PARTY_DIR}/AMICI/build")
set(AMICI_DIR "${THIRD_PARTY_DIR}/AMICI")
set(AMICI_INCLUDE_DIR "${AMICI_DIR}/include")
set(AMICI_LIB "${AMICI_DIR}/build/libamici.a")
set(BLA_VENDOR OpenBLAS)
find_package(BLAS REQUIRED)

# === includes ===
# include(ExternalProject)

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
