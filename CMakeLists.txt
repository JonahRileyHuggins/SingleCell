cmake_minimum_required(VERSION 3.10)
project(SingleCellSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Paths
set(Amici_DIR "${CMAKE_SOURCE_DIR}/libs/AMICI-master/build")

set(AMICI_DIR "${CMAKE_SOURCE_DIR}/libs/AMICI-master")
set(AMICI_INCLUDE_DIR "${AMICI_DIR}/include")
set(AMICI_LIB "${AMICI_DIR}/build/libamici.a")


# Include subdirectory-required code:
# AMICI
add_subdirectory(amici_models/Deterministic)
# Pybind11
add_subdirectory(libs/pybind11)
# libXML2
add_subdirectory(libs/libxml2)
# muparser
add_subdirectory(libs/muparser)

# Pre-baked Pybind Make:
pybind11_add_module(SingleCell src/SingleCellSimulator.cpp)

# Include headers (SGEmodule.h is in src/)
target_include_directories(SingleCellSimulator PRIVATE
    ${AMICI_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Link against AMICI and model library
target_link_libraries(SingleCellSimulator PRIVATE
    ${AMICI_LIB}
    Deterministic
)

# Set output directory (useful for locating the .so/.dylib/.dll file)
set_target_properties(SingleCellSimulator PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_custom_command(TARGET SingleCellSimulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:SingleCellSimulator>
        ${CMAKE_SOURCE_DIR}/python/
)
