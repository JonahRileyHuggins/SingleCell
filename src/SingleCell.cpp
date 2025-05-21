/**
 * @file SingleCell.cpp
 * 
 * @brief Combines instances of Deterministic and Stochastic Modules into singular simulation framework
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 */

// --------------------------Library Import--------------------------------//
#include <vector>
#include <string>
#include <unordered_set>


// Internal Libraries
#include "singlecell/SingleCell.h"
#include "singlecell/Simulation.h"
#include "singlecell/SBMLHandler.h"
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"
//-----------------------------Class Details-------------------------------//
SingleCell::SingleCell(
    const std::string& stochastic_sbml_path,
    const std::string& deterministic_sbml_path
)
    {

        std::unique_ptr<SBMLHandler> stochastic_sbml = std::make_unique<SBMLHandler>(stochastic_sbml_path);
        std::unique_ptr<SBMLHandler> deterministic_sbml = std::make_unique<SBMLHandler>(deterministic_sbml_path);
    
        this->StochasticModel = stochastic_sbml->getModel();
        this->DeterministicModel = deterministic_sbml->getModel();

    }

std::vector<std::vector<double>> SingleCell::simulate(
    const std::vector<double>& det_states, 
    const std::vector<double>& stoch_states,
    double start, 
    double stop,
    double step
) {
    /**
     * @brief public method for users to interface with the SingleCell Simulator. 
     * 
     * @param det_states are the initial species values for the deterministic AMICI model
     * @param stoch_states are the initial species values for the stochastic SBML model
     * @param start is the simulation start time
     * @param stop is the simulation stop time, in seconds
     * @param step is the delta_t step between simulation updates in seconds
     * 
     * @returns matrix of global states for both models
     */
    //Create instances of internal simulation modules: dynamic allocation
    std::unique_ptr<StochasticModule> stochMod = std::make_unique<StochasticModule>(StochasticModel);
    std::unique_ptr<DeterministicModule> detMod = std::make_unique<DeterministicModule>(DeterministicModel);

    // Add simulation time steps, results matrix, 
    stochMod->_simulationPrep(stoch_states, start, stop, step);
    detMod->_simulationPrep(det_states, start, stop, step);

    std::vector<double> timeSteps = Simulation::setTimeSteps(start, stop, step);

    // Main iterating for-loop: we're going to stop it and update vals every second until total time reached.
    for (int timestep = 0; timestep < timeSteps.size(); timestep++) {

        //Run Module Simulations
        stochMod->runStep(timestep);
        detMod->runStep(timestep);

        // exchange data
        stochMod->updateParameters(detMod->sbml);
        detMod->updateParameters(detMod->sbml);

    }
    
    // concatentate results matrices
    std::vector<std::vector<double>> results_matrix = Simulation::concatenateMatrixRows(
        stochMod->results_matrix,
        detMod->results_matrix
    );

    return results_matrix;
}

