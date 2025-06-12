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
#include "singlecell/BaseModule.h"
#include "singlecell/SBMLHandler.h"
//=============================Class Details================================//
BaseModule::BaseModule(
    SBMLHandler Module
) : handler(Module) {}

std::vector<double> BaseModule::setTimeSteps(double start, double stop, double step) {
     // Initialized array to be returned:
    std::vector<double> timepoints;

    // For loop for calculation
    for (double val = start; val < stop; val +=step) {

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
    results_matrix[timepoint] = state_vector;

    //send updated Species info to sbml:
    for (int i = 0; i < this->sbml->getNumSpecies(); i++) {

        this->sbml->getSpecies(i)->setInitialConcentration(state_vector[i]);

    }
}

void BaseModule::findOverlappingIds(
    const Model* alternate_model
) {

    std::vector<std::string> alt_species_ids;

    int numSpecies = alternate_model->getNumSpecies();

    for (int i = 0; i < numSpecies; i++) {

        const Species* species = alternate_model->getSpecies(i);

        alt_species_ids.push_back(species->getId());
    }

    std::vector<std::string> param_ids = handler.getParameterIds();

    std::unordered_set<std::string> lookup(alt_species_ids.begin(), alt_species_ids.end());

    for (const auto& id : param_ids) {
        if (lookup.count(id)) {
            this->overlapping_params.push_back(id);
        }
    }
}

void BaseModule::modifyModelEntity(
    std::string entity_id, 
    double new_value
) {
    try {
        this->handler.setModelEntityValue(entity_id, new_value);
    } catch (...) {
        
    }
}
