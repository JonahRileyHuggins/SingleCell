/*
filename: SingleCellSimulator.cpp
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
    private:
    // Create Factory Loader method here
    // Maybe a simple function for assigning timepoints per iteration?
    std::vector<double> setTimeSteps(double start, double stop, double step) {
        
        // Initialized array to be returned:
        std::vector<double> timepoints;

        // For loop for calculation
        for (int i = 0; i <= static_cast<int>((stop - start) / step); ++i) {

            timepoints.push_back(static_cast<double>(i));
        }

        // NSteps equivalent in SPARCED:
        return timepoints;
    }


    std::vector<double> setAllSpeciesValues(
                                            std::vector<double> current_states,
                                            std::vector<double> update_states
                                            ) {

        // Creating instance of list to be returned:
        std::vector<double> updated_concentrations;
        for (amici::realtype val : current_states) {

            updated_concentrations.push_back(static_cast<double>(val));

        } 

        // Initial Species modification:
        for (int i = 0; i < update_states.size(); ++i) {

            updated_concentrations[i] = update_states[i];

        }
        
        return updated_concentrations;

    }

    std::vector<double> getLastValues(const amici::ReturnData &rdata) {

        int n_species = rdata.nx; // number of species
        int n_timepoints = rdata.nt; 
        
        const std::vector<double>& all_species = rdata.x; // species trajectories flat array length n_species * n_timepoints

        int last_idx = (n_timepoints-1) * n_species;

        std::vector<double> last_species_values;
        
        // convert each species value from amici::realtype to double
        for (int i = last_idx; i < last_idx + n_species; ++i) {
            last_species_values.push_back(static_cast<double>(all_species[i]));
        
        }

        return last_species_values;
    }

    public:
    SingleCell() {

    }
    
    std::vector<std::vector<double>> simulate(std::vector<double> deterministic_initial_states, 
                                                std::vector<double> stochastic_initial_states,
                                                double start = 0.0, 
                                                double stop = 86400.0, 
                                                double step = 30.0) {
        /**
         * @brief public method for users to interface with the SingleCell Simulator. 
         * 
         * @param deterministic_initial_states are the initial species values for the deterministic AMICI model
         * @param stochastic_initial_states are the initial species values for the stochastic SBML model
         * @param start is the simulation start time
         * @param stop is the simulation stop time, in seconds
         * @param step is the delta_t step between simulation updates in seconds
         * 
         * @returns matrix of global states for both models
         */

        //--ODE--AMICI BLOCK
        std::vector<double> timesteps = setTimeSteps(start, stop, step);

        // Species array to assign results to:
        std::vector<std::vector<double>> results_array(timesteps.size(), std::vector<double>(species_ids.size()));

        //assign initial states as first set of values in results_array
        for (size_t i = 0; i < initial_states.size(); ++i) {
            results_array[0][i] = initial_states[i];
        }

        // Main iterating for-loop: we're going to stop it and update vals every second until total time reached.
        for (int i = 0; i < timesteps.size(); i++) {

            std::vector<double> step_forward = {0, step};
            ode_model->setTimepoints(step_forward);

            //set initial states based on last iteration's final values:
            std::vector<double> current_states = results_array[(i > 0) ? i - 1 : i];

            ode_model->setInitialStates(current_states);

            // Run Simulation
            std::unique_ptr<amici::ReturnData> rdata = amici::runAmiciSimulation(*solver, nullptr, *model);

            //results save step
            std::vector<double> last_vals = getLastValues(*rdata);

            // Implement SGEmodule:
            std::vector<double> stochastic_values = stochastic_module(last_vals);

            for (size_t j = 0; j < stochastic_values.size(); ++j) {

                results_array[i][j] = stochastic_values[j];

            }

        }

        return results_array;
    }

    protected:
        // Hook for data‐exchange: let children swap state as needed.
        virtual void exchangeData(
            const std::vector<double>& detState,
            std::vector<double>& stochState
        ) = 0;

    }
}

class Deterministic : private SingleCell {
    private:
        std::unique_ptr<amici::Model> ode_model;

        std::unique_ptr<amici::Solver> solver;

    public:
        Deterministic() : () {
        // Import AMICI Model from 'bin/AMICI_MODELS/model
        std::unique_ptr<amici::Model> ode_model = amici::generic_model::getModel();

        // Initial Species modification:
        std::vector<std::string> species_ids = ode_model->getStateIds();
        ode_model->setInitialStates(initial_states); 

        // Create an instance of the solver class
        std::unique_ptr<amici::Solver> solver = ode_model->getSolver();
        
        // Assign solver settings
        solver->setAbsoluteTolerance(1e-10);
        solver->setRelativeTolerance(1e-10);
        solver->setMaxSteps(10000);
        }

    protected: 
}

class Stochastic : private SingleCell {
    public:
    // Create instance of StochasticModule class
    std::unique_ptr<StochasticModule> Stochastic;

    private:

    protected: 
}


// PyBind module, syntax is "import [simulate] as [m]"
PYBIND11_MODULE(SingleCellSimulator, m) {

    m.doc() = "SingleCell runner module with stochastic gene expression support";

    m.def("simulate", &simulate,
        py::arg("initial_states"), 
        py::arg("start") = 0.0,
        py::arg("stop") = 1.0,
        py::arg("step") = 0.001,
        "Run SingleCell simulation with optimal time range"    
    );
    
}