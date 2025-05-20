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

PYBIND11_MODULE(SingleCell, m) {
    py::class_<SingleCell>(m, "SingleCell")
        .def(py::init<const std::string&, const std::string&>())  // Constructor
        .def("simulate", &SingleCell::simulate, 
        py::arg("stochastic_sbml_path") = "../sbml_files/Stochastic.sbml",
        py::arg("deterministic_sbml_path") = "../sbml_files/Deterministic.sbml",
        py::arg("start") = 0.0,
        py::arg("stop") = 60.0,
        py::arg("step") = 30.0
    );
        // JONAH-->Add more methods here as needed
}
