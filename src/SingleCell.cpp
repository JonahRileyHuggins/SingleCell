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
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"
//-----------------------------Class Details-------------------------------//
SingleCell::SingleCell(
    std::string stochastic_sbml_path,
    std::string deterministic_sbml_path
)
    : stochasticModule(std::make_unique<StochasticModule>(stochastic_sbml_path)),
      deterministicModule(std::make_unique<DeterministicModule>(deterministic_sbml_path))
{

}

SingleCell::~SingleCell() {

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
    std::unique_ptr<StochasticModule> stochMod = std::make_unique<StochasticModule>();
    std::unique_ptr<DeterministicModule> detMod = std::make_unique<DeterministicModule>();

    // Add simulation time steps, results matrix, 
    stochMod->_simulationPrep(stoch_states, start, stop, step);
    detMod->_simulationPrep(det_states, start, stop, step);

    std::vector<double> timeSteps = SingleCell::setTimeSteps(start, stop, step);

    // Main iterating for-loop: we're going to stop it and update vals every second until total time reached.
    for (int timestep = 0; timestep < timeSteps.size(); timestep++) {

        //Run Module Simulations
        stochMod->runStep(timestep);
        detMod->runStep(timestep);

        // exchange data

    }
    // concatentate results matrices
}
std::vector<double> SingleCell::setTimeSteps(double start, double stop, double step) {
     // Initialized array to be returned:
    std::vector<double> timepoints;

    // For loop for calculation
    for (int i = 0; i <= static_cast<int>((stop - start) / step); ++i) {

        timepoints.push_back(static_cast<double>(i));
    }

    // NSteps equivalent in SPARCED:
    return timepoints;
}

std::vector<std::vector<double>> SingleCell::createResultsMatrix(
    int numSpecies,
    int numTimeSteps
) {

    std::vector<std::vector<double>> results_matrix(numTimeSteps, std::vector<double>(numSpecies));

    return results_matrix;

}

std::vector<std::string> SingleCell::findOverlappingIds(
    const std::vector<std::string>& ids1,
    const std::vector<std::string>& ids2
) {
    std::vector<std::string> overlaps;
    std::unordered_set<std::string> lookup(ids2.begin(), ids2.end());

    for (const auto& id : ids1) {
        if (lookup.count(id)) {
            overlaps.push_back(id);
        }
    }

    return overlaps;
}
