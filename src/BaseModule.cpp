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

std::vector<double> BaseModule::setTimeSteps(double start, double stop, double step) {
     // Initialized array to be returned:
    std::vector<double> timepoints;

    // For loop for calculation
    for (double val = start; val < stop+step; val +=step) {

        timepoints.push_back(val);
    }

    // NSteps equivalent in SPARCED:
    return timepoints;
}

std::vector<std::vector<double>> BaseModule::createResultsMatrix(
    int numSpecies,
    int numTimeSteps
) {

    std::vector<std::vector<double>> results_matrix(numTimeSteps, std::vector<double>(numSpecies));

    return results_matrix;

}


void BaseModule::recordStepResult(
    const std::vector<double>& state_vector,
    int timepoint
) {
    this->results_matrix[timepoint] = state_vector;

}

std::vector<double> BaseModule::getSpeciesValues() {

    std::vector<double> return_list(this->species_list.size());

    for (int i = 0; i < this->species_list.size(); i++) {

        return_list[i] = this->component_map[species_list[i]];

    }
    return return_list;
}

std::vector<double> BaseModule::getParameterValues() {

    std::vector<double> return_list(this->params_list.size());

    for (int i = 0; i < this->params_list.size(); i++) {

        return_list[i] = this->component_map[params_list[i]];

    }
    return return_list;
}

std::vector<double> BaseModule::getStoreData() {

    std::vector<double> store_data(this->store.size());

    for (int i = 0; i < this->store.size(); i++) {

        store_data[i] = this->component_map[this->store[i]];

    }
    return store_data;
}

void BaseModule::updateComponentMap(
    std::vector<std::string> entities, 
    std::vector<double> updates
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
