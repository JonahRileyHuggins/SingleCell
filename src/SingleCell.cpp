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
    { "Deterministic", [](const SBMLHandler& handler) { return std::make_unique<DeterministicModule>(handler); } },
    { "Stochastic", [](const SBMLHandler& handler) { return std::make_unique<StochasticModule>(handler); } }
};

void SingleCell::loadSimulationModules() {

    for (const SBMLHandler& handler : handlers) {

        const std::string id = handler.model->getId();
        
        auto matched_module = this->moduleFactory.find(id);

        if (matched_module != moduleFactory.end()) {
            
            // Call the factory function with the SBMLHandler
            std::unique_ptr<BaseModule> base_mod = matched_module->second(handler); // <--This seems like your problem line

            // if module is empty; there's no need to add overhead:
            if (!base_mod->handler.getSpeciesIds().empty()) {

                // Move the pointer into the list of modules
                this->modules.push_back(std::move(base_mod));

            }
        } else {
            // Fallback if no match
            std::unique_ptr<BaseModule> base_mod = std::make_unique<DeterministicModule>(handler);
            this->modules.push_back(std::move(base_mod));
        }
    }
}

void SingleCell::assignGlobalTargets() {

    for (const auto& mod : this->modules) {

        mod->loadTargetModule(this->modules);

    }
}

void SingleCell::findModuleOverlaps() {

    for (const auto& mod : this->modules) {

        for (const auto& target : mod->targets) {

            mod->findOverlappingIds(target->sbml);
        
        }
    }
}

void SingleCell::setGlobalSimulationSettings(
    std::unordered_map<std::string, double>entity_map,
    double start, 
    double stop, 
    double step
) {
    for (const auto& mod : this->modules) {

        mod->setSimulationSettings(
            entity_map,
            start,
            stop,
            step
        );
    }
}

void SingleCell::runGlobalStep(
    int timestep
) {

    for (const auto& mod : this->modules) {

        mod->runStep(timestep);

    }

}

void SingleCell::updateGlobalParameters() {

    for (const auto& mod : this->modules) {

        mod->updateParameters();

    }

}

std::vector<std::vector<double>> SingleCell::concatenateMatrixRows(
    std::vector<std::vector<double>> matrix1, 
    std::vector<std::vector<double>> matrix2
) {
    for (size_t i = 0; i < matrix1.size(); ++i) {
        matrix1[i].insert(matrix1[i].end(), matrix2[i].begin(), matrix2[i].end());
    }
    return matrix1;
}

std::vector<std::vector<double>> SingleCell::makeResultsMatrix(
    int timesteps
) {

    int numSpecies = this->getGlobalSpeciesIds().size();

    std::vector<std::vector<double>> final_matrix;

    for (size_t m = 0; m < this->modules.size(); m++) {

        if (m == 0) {

            final_matrix = this->modules[m]->results_matrix;
        } else {

            std::vector<std::vector<double>> mod_matrix = this->modules[m]->results_matrix;

            for (size_t t = 0; t < mod_matrix.size(); t++) {

                final_matrix[t].insert(
                    final_matrix[t].end(),
                    mod_matrix[t].begin(),
                    mod_matrix[t].end()
                );
            }
        }
    }
    return final_matrix;
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
    this->loadSimulationModules();

    // Assign Target per Module
    this->assignGlobalTargets();

    // Identify all module overlaps between targets
    this->findModuleOverlaps();

    // Add simulation time steps, results matrix
    this->setGlobalSimulationSettings(
        entity_map,
        start,
        stop,
        step
    );

    std::vector<double> timeSteps = BaseModule::setTimeSteps(start, stop, step);

    // Main iterating for-loop: we're going to stop it and update vals every second until total time reached.
    for (int timestep = 0; timestep < timeSteps.size(); timestep++) {

        //Run Module Simulations
        this->runGlobalStep(timestep);

        // exchange data
        this->updateGlobalParameters();

        auto iter_t = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> iter_time = iter_t - start_t;
        printf("Iteration [%i / %i] Time: %f", 
                            (int)(timestep + 1), 
                            (int)(timeSteps.size()), 
                            iter_time.count());
        printf("\n");
    }
    
    // concatentate results matrices
    std::vector<std::vector<double>> results_matrix = makeResultsMatrix(
        timeSteps.size()
    );

    auto stop_t = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = stop_t - start_t;

    printf("Simulation Completed in %f seconds.", static_cast<double>(duration.count()));
    printf("\n");

    return results_matrix;
}

std::vector<std::string> SingleCell::getGlobalSpeciesIds() {

    std::vector<std::string> global_ids;

    for (const auto& mod : this->modules) {

        std::vector<std::string> mod_species_ids = mod->handler.getSpeciesIds();

        for (const auto& specie : mod_species_ids) {

            global_ids.push_back(specie);

        }
    }

    return global_ids;
}