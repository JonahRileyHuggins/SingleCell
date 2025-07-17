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

PYBIND11_MODULE(pySingleCell, m) {
    py::class_<SingleCell, py::smart_holder>(m, "SingleCell")
        /* lines 27:29 are a makeshift solution for dynamic module loading, as
        Pybind11 does not support variadic constructors. If 4+ SBML paths are 
        required; add another definition method (i.e. `.def()`) including 4+ 
        const string arguments */
        .def(py::init<const std::string&>())
        .def(py::init<const std::string&, const std::string&>())
        .def(py::init<const std::string&, const std::string&, const std::string&>())
        .def("simulate", &SingleCell::simulate, 
            py::arg("start") = 0.0,
            py::arg("stop") = 60.0,
            py::arg("step") = 30.0
        )
        .def("modify", &SingleCell::modify,
        py::arg("entity_id"), 
        py::arg("value")
        )
        .def("getGlobalSpeciesIds", &SingleCell::getGlobalSpeciesIds);
        // JONAH-->Add more methods here as needed
}
