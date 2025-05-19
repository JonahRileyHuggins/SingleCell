/**
 * @file bindings.cpp
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 19-05-2025
 * 
 * @brief Module for binding SingleCell Class for python exposure
 */

// --------------------------Library Import-----------------------------------//
// Internal Libraries
#include "singlecell/SingleCell.h"    // class header

// Third Party Libraries
#include <pybind11/stl.h>  // needed for std::vector, std::string
#include <pybind11/pybind11.h>

//
namespace py = pybind11;

PYBIND11_MODULE(singlecell_module, m) {
    py::class_<SingleCell>(m, "SingleCell")
        .def(py::init<>())  // Constructor
        .def("simulate", &SingleCell::simulate)
        .def("getInitialState", &SingleCell::getInitialState)
        // JONAH-->Add more methods here as needed
        ;
}
