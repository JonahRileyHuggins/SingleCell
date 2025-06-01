/**
 * @file bindings.cpp
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 19-05-2025
 * 
 * @brief Module for binding SingleCell Class for python exposure
 */

// --------------------------Library Import-----------------------------------//
// Standard Libraries
#include <string>
#include <unordered_map>

// Internal Libraries
#include "singlecell/SingleCell.h"    // class header

// Third Party Libraries
#include <pybind11/stl.h>  // needed for std::vector, std::string
#include <pybind11/pybind11.h>

namespace py = pybind11;

std::unordered_map<std::string, double>entity_map;

PYBIND11_MODULE(SingleCell, m) {
    py::class_<SingleCell> singlecell(m, "SingleCell");

    singlecell.def(py::init<const std::string&, const std::string&>())  // Constructor
        .def("simulate", &SingleCell::simulate, 
        py::arg("entity_map") = entity_map,
        py::arg("start") = 0.0,
        py::arg("stop") = 60.0,
        py::arg("step") = 30.0
        );
        // JONAH-->Add more methods here as needed
}
