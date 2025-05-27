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
#include <chrono>
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
    auto start_t = std::chrono::high_resolution_clock::now();
    printf("Starting Simulation for %f seconds.", stop);
    printf("\n");

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

        auto iter_t = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> iter_time = iter_t - start_t;
        printf("Iteration [%i / %i] Time: %f", 
                            (int)(timestep + 1), 
                            (int)(timeSteps.size()), 
                            iter_time.count());
        printf("\n");
    }
    
    // concatentate results matrices
    std::vector<std::vector<double>> results_matrix = Simulation::concatenateMatrixRows(
        stochMod.results_matrix,
        detMod.results_matrix
    );

    auto stop_t = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = stop_t - start_t;

    printf("Simulation Completed in %f seconds.", static_cast<double>(duration.count()));
    printf("\n");

    printf("Species Order: \n");
    
    // for (int  ) {}

    return results_matrix;
}

