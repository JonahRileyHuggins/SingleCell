/**
 * @file One4AllModule.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Definitions for One4AllModule operations
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
#include "SBMLHandler.h"
#include "One4AllModule.h"

// Third Party Libraries
#include "amici/amici.h"
#include "../amici_models/One4All/One4All.h"
#include <Eigen/Dense>

//=============================Class Details================================//
One4AllModule::One4AllModule(
    SBMLHandler One4AllModel
 ) : BaseModule(One4AllModel)
 {
    // Import AMICI Model from 'amici_models/$modelname'
    std::unique_ptr<amici::Model> new_model = std::make_unique<amici::model_One4All::Model_One4All>();
    this->model = std::move(new_model);
    
    //Update AMICI model for any modifications present in SBML:
    this->model->setFixedParameters(One4AllModel.getParameterValues());

    this->algorithm_id = One4AllModel.model->getId();
    this->source_id = "stochastic";

    //Populate Component map
    this->component_map = One4AllModel.getModelValuesMap();
    this->species_list = One4AllModel.getSpeciesIds();
    this->params_list = One4AllModel.getParameterIds();
    this->compartments_list = One4AllModel.getCompartmentIds();
    this->store = this->species_list;
}

std::string One4AllModule::getModuleId() { return this->algorithm_id; }

void One4AllModule::step(int step) {
    // Get the (step - 1)th result
    Eigen::VectorXd last_record = this->getLastStepResult(step);

    //reset SBML species values:
    this->updateComponentMap(this->species_list, last_record);
    
    // Need to update AMICI model
    this->updateAMICIModel();

    // Set the single timepoint to simulate
    std::vector<double> step_forward = {0.0, this->delta_t};

    this->model->setTimepoints(step_forward);

    // Retrieve last record state, convert to AMICI-compatible and update
    std::vector<double> last_rec_vec(last_record.data(),
                        last_record.data() + last_record.size());
    this->model->setInitialStates(last_rec_vec);

    // Run the simulation
    std::unique_ptr<amici::ReturnData> rdata = amici::runAmiciSimulation(*solver, nullptr, *model);

    // Extract results
    Eigen::VectorXd last_vals = this->getNewStepResult(*rdata);

    // Update internal state map
    this->updateComponentMap(this->species_list, last_vals);

    // Record values to results matrix
    this->recordStepResult(last_vals, step);

}

void One4AllModule::run(
    Eigen::VectorXd timepoints
) {

    // Starting vector for simulation
    Eigen::VectorXd initial_state = this->getLastStepResult(0);

    // convert eigen typesets to primitives AMICI wants
    std::vector<double> time_vec(timepoints.data(), 
                        timepoints.data() + timepoints.size());

    std::vector<double> init_state_vec(initial_state.data(),
                        initial_state.data() + initial_state.size());

    //reset SBML species values:
    this->updateComponentMap(this->species_list, initial_state);

    // Set the all timepoints for total runtime
    this->model->setTimepoints(time_vec);

    // Set AMICI object initial state
    this->model->setInitialStates(init_state_vec);

    // Run the simulation
    std::unique_ptr<amici::ReturnData> rdata = amici::runAmiciSimulation(*solver, nullptr, *model);

    int n_species = rdata->nx; // number of species
    int n_timepoints = rdata->nt; // timepoints

    for (int i = 0; i < n_timepoints; i++) {
        for (int j = 0; j < n_species; j++) {

            this->results_matrix(i, j) = rdata->x[i * n_species + j];

        }

    }
}

Eigen::VectorXd One4AllModule::getNewStepResult(
const amici::ReturnData &rdata
) {
    int n_species = rdata.nx;
    int n_timepoints = rdata.nt;

    const std::vector<double>& all_species = rdata.x;  // flat array length = n_species * n_timepoints

    int last_idx = (n_timepoints - 1) * n_species;

    Eigen::VectorXd last_species_values(n_species);

    for (int i = 0; i < n_species; ++i)
        last_species_values(i) = static_cast<double>(all_species[last_idx + i]);

    return last_species_values;
}

void One4AllModule::setSimulationSettings(
    double start, 
    double stop, 
    double step
) {

    this->delta_t = step;

    // Create an instance of the solver class
    this->solver = this->model->getSolver();

    int numSpecies = this->species_list.size();

    this->timesteps = BaseModule::setTimeSteps(start, stop, step);

    // populate results_matrix member with proper size
    this->results_matrix = BaseModule::createResultsMatrix(numSpecies, timesteps.size());

    // record initial state as first vector in results_matrix member
    BaseModule::recordStepResult(
        this->getSpeciesValues(),
        0
    );

    // Assign solver settings
    solver->setAbsoluteTolerance(1e-10);
    solver->setRelativeTolerance(1e-6);
    solver->setMaxSteps(100000);

    // Update internal state map
    this->getAltModuleStores();
    // Need to update AMICI model
    this->updateAMICIModel();
}

void One4AllModule::updateAMICIModel() {
    
    std::vector<double> param_values(this->params_list.size());

    for (int p = 0; p < this->params_list.size(); p++) {

        param_values[p] = this->component_map[this->params_list[p]];

    }
    this->model->setFixedParameters(param_values);

}

