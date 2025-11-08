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
#include "SingleCell.h"
#include "BaseModule.h"
#include "SBMLHandler.h"
#include "One4AllModule.h"
#include "StochasticModule.h"
#include "DeterministicModule.h"

// Third Party Libraries
#include <Eigen/Dense>

//=============================Class Details================================//
std::map<std::string, std::function<std::unique_ptr<BaseModule>(const SBMLHandler&)>> SingleCell::moduleFactory = {
    { "deterministic", [](const SBMLHandler& handler) { return std::make_unique<DeterministicModule>(handler); } },
    { "stochastic", [](const SBMLHandler& handler) { return std::make_unique<StochasticModule>(handler); } },
    { "One4All", [](const SBMLHandler& handler) { return std::make_unique<One4AllModule>(handler); } }
};

Eigen::MatrixXd SingleCell::simulate(
    double start, 
    double stop,
    double step
) {

    //Create instances of internal simulation modules: dynamic allocation
    this->loadSimulationModules();

    // Assign Target per Module
    this->assignGlobalSources();
    
    // Add simulation time steps, results matrix
    this->setGlobalSimulationSettings(
        start,
        stop,
        step
    );

    Eigen::VectorXd timeSteps = BaseModule::setTimeSteps(start, stop, step);

    // run simulation:
    this->runGlobal(timeSteps);

    // combine each module's results matrix together
    Eigen::MatrixXd results_matrix = combineResultsMatrix();

    this->modules.clear();

    return results_matrix;
}

void SingleCell::modify(
    std::string entity_id,
    double value
) {
    for ( auto& handler : this->handlers) {
        handler.setModelEntityValue(
            entity_id, 
            value
            );
    }
}

void SingleCell::loadSimulationModules() {

    for (const SBMLHandler& handler : handlers) {

        const std::string id = handler.model->getId();

        std::cout << "SBML Handler: " << id << "\n";

        auto matched_module = this->moduleFactory.find(id);

        if (matched_module != moduleFactory.end()) {

            // Call the factory function with the SBMLHandler
            std::unique_ptr<BaseModule> base_mod = matched_module->second(handler);

            // if module is empty; there's no need to add overhead:
            if (!base_mod->handler.getSpeciesIds().empty()) {

                // Move the pointer into the list of modules
                this->modules.push_back(std::move(base_mod));

            }
        }
    }
}

void SingleCell::assignGlobalSources() {

    for (const auto& mod : this->modules) {

        mod->loadSourceModules(this->modules);

    }
}

void SingleCell::setGlobalSimulationSettings(
    double start,
    double stop,
    double step
) {
    for (const auto& mod : this->modules) {

        mod->setSimulationSettings(
            start,
            stop,
            step
        );
    }
}

void SingleCell::runGlobal(
    Eigen::VectorXd timesteps
) { 
    auto start_t = std::chrono::high_resolution_clock::now();
    printf("Running Simulation for %lu steps.", timesteps.size());
    printf("\n");

    if (this->modules.size() == 1) {
        for (const auto& mod : this->modules) {

            std::cout << "Simulating fully " << mod->getModuleId() << "\n";

            mod->run(timesteps);

        }

    } else {
        // Main iterating for-loop: we're going to stop it and update vals every second until total time reached.
        for (int step = 1; step < timesteps.size(); step++) {

            //Run Module Simulations
            this->stepGlobal(step);

            // exchange data
            this->updateGlobalMaps();
        }
    }

    auto stop_t = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = stop_t - start_t;

    printf("Simulation Completed in %f seconds.", static_cast<double>(duration.count()));
    printf("\n");

}

void SingleCell::stepGlobal(
    int timestep
) {
    #pragma omp parallel for
    for (const auto& mod : this->modules) {
        
        mod->step(timestep);

    }

}

void SingleCell::updateGlobalMaps() {
    #pragma omp parallel for
    for (const auto& mod : this->modules) {

        mod->getAltModuleStores();

    }

}

Eigen::MatrixXd SingleCell::combineResultsMatrix() {

    if (modules.empty())
        return Eigen::MatrixXd();  // empty

    Eigen::MatrixXd final_matrix = modules[0]->results_matrix;

    for (size_t m = 1; m < modules.size(); ++m) {
        const Eigen::MatrixXd& mod_matrix = modules[m]->results_matrix;

        // Check number of rows
        assert(final_matrix.rows() == mod_matrix.rows());

        // Resize final_matrix to hold extra columns
        int oldCols = final_matrix.cols();
        final_matrix.conservativeResize(Eigen::NoChange, oldCols + mod_matrix.cols());

        // Copy new module matrix into the newly added columns
        final_matrix.block(0, oldCols, mod_matrix.rows(), mod_matrix.cols()) = mod_matrix;
    }

    return final_matrix;
}

std::vector<std::string> SingleCell::getGlobalSpeciesIds() {

    std::vector<std::string> global_ids;

    for (auto& handler : this->handlers) {

        std::vector<std::string> mod_species_ids = handler.getSpeciesIds();

        for (const auto& specie : mod_species_ids) {

            global_ids.push_back(specie);

        }
    }
    return global_ids;
}
