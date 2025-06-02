/**
 * @file DeterministicModule.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Definitions for DeterministicModule operations
*/

//===========================Library Import=================================//
//Std Libraries
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

//=============================Class Details================================//
DeterministicModule::DeterministicModule(
    SBMLHandler DeterministicModel
 ) : Simulation(DeterministicModel)
 {
    // Retrieve the Deterministic matrix from the sbml document.
    this->stoichmat = DeterministicModel.getStoichiometricMatrix();

    // List of formula strings to be parsed. <-- !!! Might swap for something ASTNode compatible later.
    this->formulas_vector = DeterministicModel.getReactionExpressions();

    //Instantiate SBML model
    this->sbml = DeterministicModel.model;

}

void DeterministicModule::runStep(int step) {
    // Get the (step - 1)th result
    std::vector<double> last_record = this->getLastStepResult(step);

    // Set the single timepoint to simulate
    std::vector<double> step_forward = {0.0, static_cast<double>(step)};

    model->setTimepoints(step_forward);

    // Set initial state based on last record
    model->setInitialStates(last_record);

    for (int i = 0; i < this->sbml->getNumParameters(); i++) {

        std::cout << "Parameter {" << this->sbml->getParameter(i)->getId()
        << "} has value: " << this->sbml->getParameter(i)->getValue() << "\n";
    }

    // Run the simulation
    std::unique_ptr<amici::ReturnData> rdata = amici::runAmiciSimulation(*solver, nullptr, *model);

    // Extract results (assuming you want the final state)
    std::vector<double> last_vals = this->getNewStepResult(*rdata);

    // Record values to results matrix
    Simulation::recordStepResult(last_vals, step);
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

std::vector<double> DeterministicModule::getNewStepResult(
    const amici::ReturnData &rdata
) {

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
    std::unordered_map<std::string, double>entity_map,
    double start, 
    double stop, 
    double step
) {
    // modify sbml model prior to AMICI-model assignment
    std::vector<double> init_states;

    if (entity_map.empty()) {
        printf("Using default model state for simulation.\n");
        init_states = handler.getInitialState();
    } else { 
        for (const auto& [key, value] : entity_map) {
            this->handler.setModelEntityValue(
                key, 
                value
            );

            init_states = handler.getInitialState();
        }
    }

     // Import AMICI Model from 'bin/AMICI_MODELS/model
    this->model = amici::generic_model::getModel();

    // Create an instance of the solver class
    this->solver = model->getSolver();

     int numSpecies = this->sbml->getNumSpecies();
          
     std::vector<double> timeSteps = Simulation::setTimeSteps(start, stop, step);

     // populate results_matrix member with proper size
     this->results_matrix = Simulation::createResultsMatrix(numSpecies, timeSteps.size()); 
     
     // record initial state as first vector in results_matrix member 
     Simulation::recordStepResult(
        init_states, 
        0
    );
     
     // Assign solver settings
     solver->setAbsoluteTolerance(1e-10);
     solver->setRelativeTolerance(1e-6);
     solver->setMaxSteps(100000);
}

std::vector<double> DeterministicModule::getLastStepResult(
    int timestep
) {

    std::vector<double> state_vector(this->results_matrix.size());

    state_vector = this->results_matrix[
        (timestep > 0) ? timestep - 1 : timestep
    ];

    return state_vector;
}

void DeterministicModule::updateParameters(
    const Model* alternate_model
) {
    std::vector<std::string> alt_species_ids;

    int numSpecies = alternate_model->getNumSpecies();

    for (int i = 0; i < numSpecies; i++) {

        const Species* species = alternate_model->getSpecies(i);

        alt_species_ids.push_back(species->getId());
    }

    std::vector<std::string> param_ids = handler.getParameterIds();
    
    std::vector<std::string> overlapping_params = Simulation::findOverlappingIds(
        param_ids, 
        alt_species_ids
    );

    for (int i = 0; i < overlapping_params.size(); i++) {

        std::cout << "alt model param: " << alternate_model->getSpecies(overlapping_params[i])->getId()
        << "val: " << alternate_model->getSpecies(overlapping_params[i])->getInitialConcentration() << "\n";
        // Deterministic model needs both AMICI and SBML set:
        //AMICI
        this->model->setFixedParameterById(overlapping_params[i], alternate_model->getSpecies(overlapping_params[i])->getInitialConcentration());
        //SBML
        this->sbml->getParameter(overlapping_params[i])->setValue(alternate_model->getSpecies(overlapping_params[i])->getInitialConcentration());
    }

}