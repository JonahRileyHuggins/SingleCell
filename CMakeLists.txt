cmake_minimum_required(VERSION 3.10)
project(SingleCell)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Paths
set(Amici_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/AMICI-master/build")

set(AMICI_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/AMICI-master")
set(AMICI_INCLUDE_DIR "${AMICI_DIR}/include")
set(AMICI_LIB "${AMICI_DIR}/build/libamici.a")

# Include subdirectory-required code:
# AMICI
add_subdirectory(amici_models/Deterministic)
# Pybind11
add_subdirectory(ThirdParty/pybind11)
# libXML2
add_subdirectory(ThirdParty/libxml2)
# libSBML
add_subdirectory(ThirdParty/libsbml-5.20.2)
# muparser
add_subdirectory(ThirdParty/muparser)

# Pre-baked Pybind Make:
pybind11_add_module(SingleCell src/SingleCell.cpp)

# Include headers (*.h are in include/singlecell)
target_include_directories(SingleCell PRIVATE
    ${AMICI_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include/
)


# Link against AMICI and model library
target_link_libraries(SingleCell PRIVATE
    ${AMICI_LIB}
    Deterministic
)

# Set output directory (useful for locating the .so/.dylib/.dll file)
set_target_properties(SingleCell PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

add_custom_command(TARGET SingleCell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:SingleCell>
        ${CMAKE_SOURCE_DIR}/python/
)
