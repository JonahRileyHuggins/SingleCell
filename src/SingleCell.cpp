/**
 * @file SingleCell.cpp
 * 
 * @brief Combines instances of Deterministic and Stochastic Modules into singular simulation framework
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 */

//===========================Library Import=================================//
//Std Libraries
#include <vector>
#include <string>
#include <unordered_set>


// Internal Libraries
#include "singlecell/SingleCell.h"
#include "singlecell/Simulation.h"
#include "singlecell/SBMLHandler.h"
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"

//=============================Class Details================================//
SingleCell::SingleCell(
    const std::string& stochastic_sbml_path,
    const std::string& deterministic_sbml_path
) : StochasticModel(stochastic_sbml_path),
    DeterministicModel(deterministic_sbml_path)
    { }

std::vector<std::vector<double>> SingleCell::simulate(
    const std::vector<double>& det_states, 
    const std::vector<double>& stoch_states,
    double start, 
    double stop,
    double step
) {

    //Create instances of internal simulation modules: dynamic allocation
    StochasticModule stochMod = StochasticModule(StochasticModel);
    DeterministicModule detMod = DeterministicModule(DeterministicModel);

    // Add simulation time steps, results matrix, 
    stochMod._simulationPrep(stoch_states, start, stop, step);
    detMod._simulationPrep(det_states, start, stop, step);

    std::vector<double> timeSteps = Simulation::setTimeSteps(start, stop, step);

    // Main iterating for-loop: we're going to stop it and update vals every second until total time reached.
    for (int timestep = 0; timestep < timeSteps.size(); timestep++) {

        //Run Module Simulations
        stochMod.runStep(timestep);
        detMod.runStep(timestep);

        // exchange data
        stochMod.updateParameters(detMod.sbml);
        detMod.updateParameters(stochMod.sbml);

    }
    
    // concatentate results matrices
    std::vector<std::vector<double>> results_matrix = Simulation::concatenateMatrixRows(
        stochMod.results_matrix,
        detMod.results_matrix
    );

    printf("Final Matrix dimensions: [%lu, %lu]", results_matrix.size(), results_matrix[0].size());
    printf("\n");

    printf("Simulation Complete");
    printf("\n");

    return results_matrix;
}

