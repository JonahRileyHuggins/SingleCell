/*
filename: Simulator.cpp
created by: Jonah R. Huggins
created on: 25-04-12

description: 
File for orchestrating communication between the AMICI simulation function and a custom
simple stochastic gene expression routine for modifying SBML results and passing back.
*/

// --------------------------Library Import-----------------------------------//

#include <vector>
#include <memory>
#include <fstream>
#include <iostream>

// Internally written libraries
#include "stochastic_module.h"

// Third Party Libraries
#include "pybind11/stl.h"
#include "pybind11/pybind11.h"
#include "../libs/AMICI-master/include/amici/amici.h"
#include "../amici_models/Deterministic/wrapfunctions.h"

//-------------------------Namespace Declarations-----------------------------//
namespace py = pybind11;

//--------------------------Class Declarations-----------------------------//

class SingleCell {
    protected:
        // Hook for data‐exchange: let children swap state as needed.
        virtual void exchangeData(
            const std::vector<double>& detState,
            std::vector<double>& stochState
        ) = 0;

        std::vector<double> makeTimepoints(double start, double stop, double step);
        std::vector<double> getLastValues(const amici::ReturnData& rdata);


    public:    
        std::vector<std::vector<double>> simulate(
            const std::vector<double>& det_init,
            const std::vector<double>& stoch_init,
            double start, double stop, double step
        );
    }

class Deterministic : public SingleCell {
    private:
        std::unique_ptr<amici::Model> model_;
        std::unique_ptr<amici::Solver> solver_;

        std::vector<double> stepDeterministic(
            const std::vector<double>& current,
            double t0, double t1
        );

        // synchronize child input/outputs:
        void exchangeData(
            const std::vector<double>& detState, 
            std::vector<double>& stochState
        ) override {
            // TODO: For override of stochastic state initial conditions
            model_.setModelState(stochState);
        }

    public:
        Deterministic(/* add potential config args here*/);
}

class Stochastic : public SingleCell {
    private:
        StochasticModule stochMod_;

        std::vector<double> stepStochastic(
            const std::vector<double>& currrent, 
            double dt
        );

        void exchangeData(
            const std::vector<double>& detState,
            std::vector<double>& stochState
        ) override {
            // Copy deterministic state into StochMod_ as new initials
            stochMod_.setModelState(detState);
        }

    public:
        Stochastic(/* TODO: Configs like SBML file paths, parser settings, etc.*/);
}


// PyBind module, syntax is "import [simulate] as [m]"
PYBIND11_MODULE(Simulator, m) {

    m.doc() = "SingleCell runner module with stochastic gene expression support";

    m.def("simulate", &simulate,
        py::arg("initial_states"), 
        py::arg("start") = 0.0,
        py::arg("stop") = 1.0,
        py::arg("step") = 0.001,
        "Run SingleCell simulation with optimal time range"    
    );
    
}