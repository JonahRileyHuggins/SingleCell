/**
 * @file StochasticModule.h
 *
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 *
 * @brief Definitions for StochasticModule operations
*/

//===========================Library Import=================================//
//Std Libraries
#include <ctime>
#include <cmath>
#include <vector>
#include <cctype>
#include <string>
#include <random>
#include <memory>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <optional>
#include <iostream>
#include <algorithm>
#include <unordered_map>

// Internal libraries
#include "unit_conversions.h"
#include "SBMLHandler.h"
#include "StochasticModule.h"

// external library
#include "parser.h"
#include <Eigen/Dense>

//=============================Class Details================================//
StochasticModule::StochasticModule(
    SBMLHandler StochasticModel
) : BaseModule(StochasticModel) {

    // Retrieve the stoichiometric matrix from the sbml document.
    this->stoichmat = StochasticModel.getStoichiometricMatrix();

    // List of formula strings to be parsed.
    this->formulas_vector = StochasticModel.getReactionExpressions();
    this->tokenized_formula_map = StochasticModel.tokenizeFormulas();

    // Initialize Eigen variables before simulation:
    this->propensities.resize(this->formulas_vector.size());
    this->realizations.resize(this->formulas_vector.size());

    //call conversion method here:
    this->nM2mpv_conversion_factors = unit_conversions::nanomolar2mpv(StochasticModel.species_volumes);
    this->molecules2nM_conversion_factors = unit_conversions::molecules2nanomolar(StochasticModel.species_volumes);

    this->algorithm_id = StochasticModel.model->getId();
    this->source_id = "deterministic";

    //Populate Component map
    this->component_map = StochasticModel.getModelValuesMap();
    this->species_list = StochasticModel.getSpeciesIds();
    this->params_list = StochasticModel.getParameterIds();
    this->compartments_list = StochasticModel.getCompartmentIds();
    this->species_volumes = StochasticModel.species_volumes;
    this->store = this->species_list;

    this->new_state.resize(this->species_list.size());

    // Initialize random sampler only once
    std::random_device rd;
    this->generator.seed(rd());

 }

std::string StochasticModule::getModuleId() { return this->algorithm_id; }

void StochasticModule::computeReactions() {

    // Populate the vector:
    for (unsigned int i = 0; i < this->propensities.size(); i++)
        this->propensities(i) = computeReaction(this->formulas_vector[i]);
}
    
double StochasticModule::computeReaction(const std::string &formula_str) {

    // Get variables in formula
    std::unordered_map<std::string,double> components = this->getFormulaValues(formula_str);

    // Copy formula string for safe replacement
    std::string new_formula_str = formula_str;

    try {
        for (const auto& [name, value] : components) {
            new_formula_str = safe_replace_alnumus(new_formula_str, name, value);
        }

        // Send to parser algorithm

        double v_i = parser(new_formula_str.c_str()); //!<-- verify for units bug
        return v_i;
    }
    catch (const std::exception& e) {
        std::cerr << "Error computing reaction from formula '"
                  << formula_str << "': " << e.what() << std::endl;
        throw; 
    }
    catch (...) {
        std::cerr << "Unknown error computing reaction from formula '"
                  << formula_str << "'." << std::endl;
        throw;
    }
}

std::unordered_map<std::string,double> StochasticModule::getFormulaValues(
    const std::string& formula_str
) {

    std::unordered_map<std::string, double> formula_value_map;

    std::vector<std::string> components_vector = this->tokenized_formula_map[formula_str];
    // Iterate over each component and return SBML components with values associated
    for (int i = 0; i < components_vector.size(); i++) {
        formula_value_map[components_vector[i]] = this->component_map[components_vector[i]];
    
    }
    return formula_value_map;
}

bool StochasticModule::is_alnumus(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::string StochasticModule::safe_replace_alnumus(
    std::string &input,
    const std::string &swap,
    double with_val
) {
    if (swap.empty()) return input;

    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.15f", with_val);
    std::string with(buffer);

    size_t pos = 0;
    while ((pos = input.find(swap, pos)) != std::string::npos) {
        bool left_ok = (pos == 0) || !is_alnumus(input[pos - 1]);
        bool right_ok = (pos + swap.size() >= input.size()) ||
                        !is_alnumus(input[pos + swap.size()]);

        if (left_ok && right_ok) {
            input.replace(pos, swap.size(), with);
            pos += with.size();
        } else {
            pos += swap.size();
        }
    }
    return input;
}

void StochasticModule::samplePoisson() {

    for (size_t i = 0; i < this->propensities.size(); ++i) {

        std::poisson_distribution<int> dist((this->propensities(i) * this->delta_t)); 
        this->realizations(i) = dist(this->generator);

    }
}

void StochasticModule::constrainTau(
    const Eigen::VectorXd &last_state
) {
    // reset out from prior step

    const int numCols = this->stoichmat.cols();

    Eigen::ArrayXd tmp(this->realizations.size());

    for (int j = 0; j < numCols; ++j) {

        // compute coefficient products directly from the column
        tmp = (last_state.array() * this->stoichmat.col(j).array()).abs();
        const double masked_min = (tmp > 0.0).select(tmp, this->inf).minCoeff();

        // Compute min valid reactant or fallback to propensity-j
        const double R_mi = std::isfinite(masked_min) ? masked_min : this->realizations(j);
        this->realizations(j) = std::min(this->realizations(j), R_mi);
    }
}

Eigen::VectorXd StochasticModule::computeNewState(
    Eigen::VectorXd &last_state
) {
    // Update the stochastic state vector: new_state = old_state * v
    new_state.noalias() = last_state;
    new_state.noalias() += stoichmat * realizations;
    new_state = new_state.array().round();
    return new_state;
}

void StochasticModule::setSimulationSettings(
    double start, 
    double stop, 
    double step
) {

    this->delta_t = step;

    int numSpecies = this->species_list.size();

    this->timesteps = BaseModule::setTimeSteps(start, stop, step);

    this->results_matrix = BaseModule::createResultsMatrix(
        numSpecies, timesteps.size()
    );

    BaseModule::recordStepResult(
        this->getSpeciesValues(), 
        0
    );

}

void StochasticModule::step(
    int step
) {

    // get (step minus 1) position in results_matrix member
    Eigen::VectorXd last_state = this->getLastStepResult(step);  // nM

    // convert units to molecule per litre
    last_state.array() *= this->nM2mpv_conversion_factors.array(); //nM -> molecules/L
    this->updateComponentMap(this->species_list, last_state);

    // overwrite this->propensities with new step values
    this->computeReactions();

    // overwrite this->realizations by sampling from Poisson distribution
    this->samplePoisson();

    // convert back to total molecules in-place
    last_state.array() *= this->species_volumes.array();  // molecules/L -> molecules
    
    // Calculate the updated state for current step:
    Eigen::VectorXd new_state = computeNewState(last_state);
    if (new_state.minCoeff() < 0.0) {
        // Constrain Tau-leap algorithm for conservation of moiety
        this->constrainTau(last_state);
        new_state=computeNewState(last_state);
    }
    // convert units to nanoMolar
    new_state.array() *= this->molecules2nM_conversion_factors.array(); // molecules -> nM

    // Convert map values back to nanomolar value
    this->updateComponentMap(this->species_list, new_state);

    //Record iteration's result
    BaseModule::recordStepResult(new_state, step);

}

void StochasticModule::run(
    Eigen::VectorXd timesteps
) {
    for (int t = 0; t < timesteps.size(); t++) {

        this->step(t);

    }
}
