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
#include "singlecell/utils.h"
#include "singlecell/SingleCell.h"
#include "singlecell/BaseModule.h"
#include "singlecell/SBMLHandler.h"
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"

//=============================Class Details================================//
std::map<std::string, std::function<std::unique_ptr<BaseModule>(const SBMLHandler&)>> SingleCell::moduleFactory = {
    { "Deterministic", [](SBMLHandler handler) { return std::make_unique<DeterministicModule>(handler); } },
    { "Stochastic", [](SBMLHandler handler) { return std::make_unique<StochasticModule>(handler); } }
};

void SingleCell::loadSimulationModules() {

    for (const SBMLHandler& handler : handlers) {

        const std::string id = handler.model->getId();

        auto matched_module = this->moduleFactory.find(id);

        if (matched_module != moduleFactory.end()) {

            // Call the factory function with the SBMLHandler
            std::unique_ptr<BaseModule> base_mod = matched_module->second(handler);

            // Move the pointer into the list of modules
            this->modules.push_back(std::move(base_mod));
        } else {
            // Fallback if no match
            std::unique_ptr<BaseModule> base_mod = std::make_unique<DeterministicModule>(handler);
            this->modules.push_back(std::move(base_mod));
        }
    }
}

std::vector<std::vector<double>> SingleCell::simulate(
    std::unordered_map<std::string, double> entity_map,
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

    // Assign Target Overlaps per Module:
    stochMod.findOverlappingIds(detMod.sbml);
    detMod.findOverlappingIds(stochMod.sbml);

    // Add simulation time steps, results matrix, 
    stochMod._simulationPrep(entity_map, start, stop, step);
    detMod._simulationPrep(entity_map, start, stop, step);

    std::vector<double> timeSteps = BaseModule::setTimeSteps(start, stop, step);

    // Main iterating for-loop: we're going to stop it and update vals every second until total time reached.
    for (int timestep = 0; timestep < timeSteps.size(); timestep++) {

        //Run Module Simulations
        stochMod.runStep(timestep);
        detMod.runStep(timestep);

        // exchange data
        stochMod.updateParameters(detMod.handler);
        detMod.updateParameters(stochMod.handler);

        auto iter_t = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> iter_time = iter_t - start_t;
        printf("Iteration [%i / %i] Time: %f", 
                            (int)(timestep + 1), 
                            (int)(timeSteps.size()), 
                            iter_time.count());
        printf("\n");
    }
    
    // concatentate results matrices
    std::vector<std::vector<double>> results_matrix = BaseModule::concatenateMatrixRows(
        stochMod.results_matrix,
        detMod.results_matrix
    );

    auto stop_t = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = stop_t - start_t;

    printf("Simulation Completed in %f seconds.", static_cast<double>(duration.count()));
    printf("\n");

    return results_matrix;
}

std::vector<std::string> SingleCell::getGlobalSpeciesIds() {

    std::vector<std::string> global_ids = this->StochasticModel.getSpeciesIds();

    std::vector<std::string> deterministic_ids = this->DeterministicModel.getSpeciesIds();

    global_ids.insert(global_ids.end(), deterministic_ids.begin(), deterministic_ids.end());

    return global_ids;
}