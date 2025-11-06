/**
 * @file BaseModule.cpp
 * 
 * @brief Combines instances of Simulation Modules into singular simulation framework
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 19-05-2025
 */

//===========================Library Import=================================//
//Std Libraries
#include <vector>
#include <string>
#include <memory>
#include <unordered_set>


// Internal Libraries
#include "BaseModule.h"
#include "SBMLHandler.h"

// Third-Party Libraries
#include <Eigen/Dense>

//=============================Class Details================================//
BaseModule::BaseModule(
    SBMLHandler Module
) : handler(Module) {}

void BaseModule::loadSourceModules(
    const std::vector<std::unique_ptr<BaseModule>>& module_list
) {
    for (const auto& mod : module_list) {

        if (mod->getModuleId() == this->source_id) {

            this->sources.push_back(mod.get());
        }

    }
}

Eigen::VectorXd BaseModule::setTimeSteps(double start, double stop, double step) {
    // Compute number of steps (inclusive of stop)
    int n_steps = static_cast<int>(std::floor((stop - start) / step)) + 1;

    Eigen::VectorXd timepoints(n_steps);

    for (int i = 0; i < n_steps; ++i) {
        timepoints(i) = start + i * step;
    }

    return timepoints;
}


Eigen::MatrixXd BaseModule::createResultsMatrix(
    int numSpecies,
    int numTimeSteps
) {

    Eigen::MatrixXd results_matrix(numTimeSteps, std::vector<double>(numSpecies));

    return results_matrix;

}


void BaseModule::recordStepResult(
    const Eigen::VectorXd& state_vector,
    int timepoint
) {
    this->results_matrix[timepoint] = state_vector;

}

Eigen::VectorXd BaseModule::getSpeciesValues() {

    Eigen::VectorXd return_list(this->species_list.size());

    for (int i = 0; i < this->species_list.size(); i++) {

        return_list[i] = this->component_map[species_list[i]];

    }
    return return_list;
}

Eigen::VectorXd BaseModule::getParameterValues() {

    Eigen::VectorXd return_list(this->params_list.size());

    for (int i = 0; i < this->params_list.size(); i++) {

        return_list[i] = this->component_map[params_list[i]];

    }
    return return_list;
}

Eigen::VectorXd BaseModule::getStoreData() {

    Eigen::VectorXd store_data(this->store.size());

    for (int i = 0; i < this->store.size(); i++) {

        store_data[i] = this->component_map[this->store[i]];

    }
    return store_data;
}

void BaseModule::updateComponentMap(
    std::vector<std::string> entities, 
    Eigen::VectorXd updates
) {
    // Safety check ensuring values properly match entities
    assert(entities.size() == updates.size());

    for (int i = 0; i < entities.size(); i++) {

        this->component_map[entities[i]] = updates[i];

    }
}

void BaseModule::getAltModuleStores() {

    for (const auto& alt_module : this->sources) {

        /* retrieves alternate module store and updates 
        this module's component map with the respective values
        */
        this->updateComponentMap(
            alt_module->store,
            alt_module->getStoreData()
        );
    }
}

Eigen::VectorXd BaseModule::getLastStepResult(int timestep) {
    // Clamp timestep to valid index
    int t = (timestep > 0) ? timestep - 1 : timestep;

    // Extract the row as a vector
    return results_matrix.row(t).transpose();  // transpose() gives a column vector
}