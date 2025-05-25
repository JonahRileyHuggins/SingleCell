cmake_minimum_required(VERSION 3.14)
project(SingleCell)

include(ExternalProject)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# === Paths ===
set(Amici_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/AMICI-master/build")
set(AMICI_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/AMICI-master")
set(AMICI_INCLUDE_DIR "${AMICI_DIR}/include")
set(AMICI_LIB "${AMICI_DIR}/build/libamici.a")

# === External Projects ===

# --- Bison ---
ExternalProject_Add(bison_ext
    PREFIX ${CMAKE_BINARY_DIR}/bison
    URL https://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.gz
    URL_HASH SHA256=1fd56e5f5b6593e174fb4680804a57a62fe1a3ce010662ab7f1c93f2ca4c28fc
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make -j
    INSTALL_COMMAND make install
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)
set(BISON_EXECUTABLE "${CMAKE_BINARY_DIR}/bison/src/bison_ext-install/bin/bison" CACHE FILEPATH "Path to locally built Bison")

# --- PCRE2 ---
ExternalProject_Add(pcre2_ext
    PREFIX ${CMAKE_BINARY_DIR}/pcre2
    URL https://github.com/PhilipHazel/pcre2/releases/download/pcre2-10.45/pcre2-10.45.tar.gz
    URL_HASH SHA256=4a9dff4b9b0bd4adfc8486d452872cdd9f2f5e64f2d90e04f2161f85c62b0c5e
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DPCRE2_BUILD_PCRE2_8=ON
        -DPCRE2_BUILD_PCRE2_16=OFF
        -DPCRE2_BUILD_PCRE2_32=OFF
        -DPCRE2_BUILD_TESTS=OFF
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    # BUILD_ALWAYS = ON
    BUILD_COMMAND $(MAKE)
    INSTALL_COMMAND $(MAKE) install
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)
set(PCRE2_DIR "${CMAKE_BINARY_DIR}/pcre2/src/pcre2_ext-install/lib/cmake/PCRE2")

# --- libxml2 ---
ExternalProject_Add(libxml2_ext 
    PREFIX ${CMAKE_BINARY_DIR}/libxml2
    URL https://download.gnome.org/sources/libxml2/2.10/libxml2-2.10.3.tar.xz
    URL_HASH SHA256=5d2cc3d78bec3dbe212a9d7fa629ada25a7da928af432c93060ff5c17ee28a9c
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DBUILD_SHARED_LIBS=OFF
        -DLIBXML2_WITH_PYTHON=OFF
        -DLIBXML2_WITH_LZMA=OFF
        -DLIBXML2_WITH_ZLIB=OFF
        -DLIBXML2_WITH_TESTS=OFF
        -DLIBXML2_CONFIGURE_PACKAGE=ON 
    BUILD_COMMAND $(cmake --build)
    INSTALL_COMMAND $(cmake) install
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)
set(LIBXML2_DIR "${CMAKE_BINARY_DIR}/libxml2/src/libxml2_ext-install/lib/cmake/LibXml2")

# === Update CMAKE_PREFIX_PATH so find_package can work ===
set(CMAKE_PREFIX_PATH "${PCRE2_DIR}" "${LIBXML2_DIR}" ${CMAKE_PREFIX_PATH})

# === Force find_package after install dirs are defined ===
find_package(PCRE2 REQUIRED)
find_package(LibXml2 REQUIRED)

message(STATUS "Found PCRE2: ${PCRE2_INCLUDE_DIRS} ${PCRE2_LIBRARIES}")
message(STATUS "Found LibXml2: ${LIBXML2_INCLUDE_DIR} ${LIBXML2_LIBRARIES}")

# === Source Files ===
set(SINGLECELL_SRC_LIST
    src/SBMLHandler.cpp
    src/Simulation.cpp
    src/DeterministicModule.cpp
    src/StochasticModule.cpp
    src/SingleCell.cpp
    src/main.cpp
)

# === External Dependencies ===
add_subdirectory(amici_models/Deterministic)
add_subdirectory(ThirdParty/muparser)
add_subdirectory(ThirdParty/swig)
add_dependencies(swig bison_ext pcre2_ext libxml2_ext)
add_subdirectory(ThirdParty/libsbml-5.20.2)
add_subdirectory(ThirdParty/pybind11)

# === Executable Target ===
add_executable(${PROJECT_NAME} ${SINGLECELL_SRC_LIST})

target_include_directories(SingleCell PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${AMICI_INCLUDE_DIR}
)

target_link_libraries(SingleCell PRIVATE
    ${AMICI_LIB}
    Deterministic
    PCRE2::PCRE2
    swig
    LibXml2::LibXml2
    sbml
    muparser
    pybind11::module
)

add_dependencies(SingleCell pcre2_ext libxml2_ext)

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

set_target_properties(pySingleCell PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python
)

add_dependencies(pySingleCell pcre2_ext libxml2_ext)

# === Post-Build: Copy executable ===
add_custom_command(TARGET SingleCell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:SingleCell>
        ${CMAKE_SOURCE_DIR}/src/
)
