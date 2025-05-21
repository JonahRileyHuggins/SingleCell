cmake_minimum_required(VERSION 3.10)
project(SingleCell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# === Paths ===
set(Amici_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/AMICI-master/build")
set(AMICI_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/AMICI-master")
set(AMICI_INCLUDE_DIR "${AMICI_DIR}/include")
set(AMICI_LIB "${AMICI_DIR}/build/libamici.a")

# === Source Files ===
set(SINGLECELL_SRC_LIST
    src/SBMLHandler.cpp
    src/Simulation.cpp
    src/DeterministicModule.cpp
    src/StochasticModule.cpp
    src/SingleCell.cpp
    src/main.cpp)

# === External dependencies ===
add_subdirectory(amici_models/Deterministic)
add_subdirectory(ThirdParty/pybind11)
add_subdirectory(ThirdParty/libxml2)
add_subdirectory(ThirdParty/libsbml-5.20.2)
add_subdirectory(ThirdParty/muparser)

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
    sbml
    muparser
    pybind11::module  # or appropriate target exported by pybind11
    xml2              # if using libxml2
)


# Copy executable to python directory after build
add_custom_command(TARGET SingleCell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:SingleCell>
        ${CMAKE_SOURCE_DIR}/src/
)

# === Pybind11 module ===
pybind11_add_module(pySingleCell src/bindings.cpp)

target_include_directories(pySingleCell PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${AMICI_INCLUDE_DIR}
)

target_link_libraries(pySingleCell PRIVATE
    ${AMICI_LIB}
    Deterministic
)

# Output directory for .so / .pyd
set_target_properties(pySingleCell PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python
)
