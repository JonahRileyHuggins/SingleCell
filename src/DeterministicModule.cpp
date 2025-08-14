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
#include "singlecell/utils.h"
#include "singlecell/SBMLHandler.h"
#include "singlecell/DeterministicModule.h"

// Third Party Libraries
#include "amici/amici.h"
#include "../amici_models/deterministic/deterministic.h"

//=============================Class Details================================//
DeterministicModule::DeterministicModule(
    SBMLHandler DeterministicModel
 ) : BaseModule(DeterministicModel)
 {
    // Retrieve the Deterministic matrix from the sbml document.
    this->stoichmat = DeterministicModel.getStoichiometricMatrix();

    // List of formula strings to be parsed. <-- !!! Might swap for something ASTNode compatible later.
    this->formulas_vector = DeterministicModel.getReactionExpressions();

    //Instantiate SBML model
    this->sbml = DeterministicModel.model;

    // // Import AMICI Model from 'bin/AMICI_MODELS/model
    // this->model = std::make_unique<amici::model_Deterministic::Model_Deterministic>();
    std::unique_ptr<amici::Model> new_model = std::make_unique<amici::model_Deterministic::Model_Deterministic>();
    this->model = std::move(new_model);
    
    //Update AMICI model for any modifications present in SBML:
    this->model->setFixedParameters(DeterministicModel.getParameterValues());

    this->algorithm_id = this->sbml->getId();
    this->target_id = "Stochastic";
}

std::string DeterministicModule::getModuleId() { return this->algorithm_id; }

void DeterministicModule::loadTargetModule(
    const std::vector<std::unique_ptr<BaseModule>>& module_list
) {
    for (const auto& mod : module_list) {

        if (mod->getModuleId() == this->target_id) {

            this->targets.push_back(mod.get());
        }

    }
}

void DeterministicModule::step(int step) {
    // Get the (step - 1)th result
    std::vector<double> last_record = this->getLastStepResult(step);

    //reset SBML species values:
    this->handler.setState(last_record);

    // Set the single timepoint to simulate
    std::vector<double> step_forward = {0.0, this->delta_t};

    model->setTimepoints(step_forward);

    // Set initial state based on last record
    model->setInitialStates(last_record);

    // Run the simulation
    std::unique_ptr<amici::ReturnData> rdata = amici::runAmiciSimulation(*solver, nullptr, *model);

    // Extract results (assuming you want the final state)
    std::vector<double> last_vals = this->getNewStepResult(*rdata);

    this->handler.setState(last_vals);

    // Record values to results matrix
    BaseModule::recordStepResult(last_vals, step);

}

void DeterministicModule::run(
    std::vector<double> timepoints
) {

    // Starting vector for simulation
    std::vector<double> initial_state = this->getLastStepResult(0);

    //reset SBML species values:
    this->handler.setState(initial_state);

    // Set the all timepoints for total runtime
    model->setTimepoints(timepoints);

    // Set AMICI object initial state
    model->setInitialStates(initial_state);

    // Run the simulation
    std::unique_ptr<amici::ReturnData> rdata = amici::runAmiciSimulation(*solver, nullptr, *model);

    int n_species = rdata->nx; // number of species
    int n_timepoints = rdata->nt; //timepoints

    for (int i = 0; i < n_timepoints; i++) {
        for (int j = 0; j < n_species; j++) {

            this->results_matrix[i][j] = rdata->x[i * n_species + j];

        }

    }
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

    // this makes the start value of  our for-loop at rdata.x[-1, starting_column] in python terms
    int last_idx = (n_timepoints-1) * n_species;

    std::vector<double> last_species_values;
    
    // convert each species value from amici::realtype to double
    for (int i = last_idx; i < last_idx + n_species; ++i) {

        last_species_values.push_back(static_cast<double>(all_species[i]));
    
    }

    return last_species_values;
}

void DeterministicModule::setSimulationSettings(
    double start, 
    double stop, 
    double step
) {

    this->delta_t = step;

    // Create an instance of the solver class
    this->solver = this->model->getSolver();

    int numSpecies = this->sbml->getNumSpecies();

    this->timesteps = BaseModule::setTimeSteps(start, stop, step);

    // populate results_matrix member with proper size
    this->results_matrix = BaseModule::createResultsMatrix(numSpecies, timesteps.size());

    // record initial state as first vector in results_matrix member
    BaseModule::recordStepResult(
        this->handler.getInitialState(),
        0
    );

    // Assign solver settings
    solver->setAbsoluteTolerance(1e-10);
    solver->setRelativeTolerance(1e-6);
    solver->setMaxSteps(100000);

    this->updateParameters();
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

void DeterministicModule::updateParameters() {
    
    for (const auto& module : this->targets) {

        SBMLHandler alternate_model = module->handler;

        for (int i = 0; i < this->overlapping_params.size(); i++) {

            // Deterministic model needs both AMICI and SBML set:
            //AMICI
            this->model->setFixedParameterById(
                this->overlapping_params[i], 
                alternate_model.model->getSpecies(this->overlapping_params[i])->getInitialConcentration()
            );

            //SBML
            this->sbml->getParameter(this->overlapping_params[i])->setValue(
                alternate_model.model->getSpecies(this->overlapping_params[i])->getInitialConcentration()
            );
        }

    }
}
