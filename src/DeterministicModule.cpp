/**
 * @file DeterministicModule.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Definitions for DeterministicModule operations
*/

// --------------------------Library Import-----------------------------------//

#include <ctime>
#include <vector>
#include <string>
#include <random>
#include <memory>
#include <fstream>
#include <optional>
#include <iostream>
#include <algorithm>
#include <unordered_map>

// Internal libraries
#include "singlecell/SBMLHandler.h"
#include "singlecell/DeterministicModule.h"

// Third Party Libraries
#include "amici/amici.h"
#include "../amici_models/Deterministic/wrapfunctions.h"

//-----------------------------Method Details-----------------------------//
DeterministicModule::DeterministicModule(
    const std::string& sbml_path
) : SingleCell(),
    sbmlHandler(std::make_unique<SBMLHandler>(sbml_path))
 {
    // Retrieve the stoichiometric matrix from the sbml document.
    this->stoichmat = sbmlHandler->getStoichiometricMatrix();

    // List of formula strings to be parsed. <-- !!! Might swap for something ASTNode compatible later.
    this->formulas_vector = sbmlHandler->getReactionExpressions();

     // Import AMICI Model from 'bin/AMICI_MODELS/model
    this->model = amici::generic_model::getModel();

    // Create an instance of the solver class
    this->solver = model->getSolver();

}

std::vector<double> DeterministicModule::runStep(
            const std::vector<double>& state_vector
        ) {

}

std::vector<double> DeterministicModule::setAllSpeciesValues(
                                        std::vector<double> current_states,
                                        std::vector<double> update_states
                                        ) {

    // Creating instance of list to be returned:
    std::vector<double> updated_concentrations;
    for (amici::realtype val : current_states) {

        updated_concentrations.push_back(static_cast<double>(val));

    } 

    // Initial Species modification:
    for (int i = 0; i < update_states.size(); ++i) {

        updated_concentrations[i] = update_states[i];

    }
    
    return updated_concentrations;

}

std::vector<double> getLastValues(const amici::ReturnData &rdata) {

    int n_species = rdata.nx; // number of species
    int n_timepoints = rdata.nt; 
    
    const std::vector<double>& all_species = rdata.x; // species trajectories flat array length n_species * n_timepoints

    int last_idx = (n_timepoints-1) * n_species;

    std::vector<double> last_species_values;
    
    // convert each species value from amici::realtype to double
    for (int i = last_idx; i < last_idx + n_species; ++i) {
        last_species_values.push_back(static_cast<double>(all_species[i]));
    
    }

    return last_species_values;
}

void DeterministicModule::_simulationPrep(
    const std::optional<std::vector<double>>& initial_state,
    double start, 
    double stop, 
    double step
) {
     const std::vector<double>& det_states = 
     initial_state.has_value() ? initial_state.value() 
                                   : this->sbmlHandler->getInitialState();

   
     int numSpecies = this->sbmlHandler->getModel()->getNumSpecies();
     
     std::vector<double> timeSteps = SingleCell::setTimeSteps(start, stop, step);

     this->results_matrix = SingleCell::createResultsMatrix(numSpecies, timeSteps.size()); 
     
     DeterministicModule::recordStepResult(
        det_states, 
        0
    );
     
     // Assign solver settings
     solver->setAbsoluteTolerance(1e-10);
     solver->setRelativeTolerance(1e-10);
     solver->setMaxSteps(10000);

}

void DeterministicModule::recordStepResult(
    const std::vector<double>& state_vector,
    int timepoint
) {
    this->results_matrix[timepoint] = state_vector;
}

std::vector<double> DeterministicModule::getInitialState() const {
    return sbmlHandler->getInitialState();
}