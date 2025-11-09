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
#include <omp.h>

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

    // Initialize random sampler only once
    std::random_device rd;
    this->generator.seed(rd());

 }

std::string StochasticModule::getModuleId() { return this->algorithm_id; }

Eigen::VectorXd StochasticModule::computeReactions() {
    /** 
     * @brief Computes all reactions in the SBML model
     * 
     * @returns v vector of state values after initial stochiometric calculations
    */
    
    unsigned int numReactions = this->formulas_vector.size();

    Eigen::VectorXd v(numReactions);

    // Populate the matrix:
    for (unsigned int i = 0; i < numReactions; i++)
        v(i) = computeReaction(formulas_vector[i]);

    
    return v;
}
    
double StochasticModule::computeReaction(const std::string &formula_str) {

    // Get variables in formula
    std::unordered_map<std::string, double> components = this->getFormulaValues(formula_str);

    // Copy formula string for safe replacement
    std::string new_formula_str = formula_str;

    try {
        for (const auto& [name, value] : components) {
            new_formula_str = safe_replace_alnumus(new_formula_str, name, to_str(value));
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

    std::vector<std::string> components_vector = tokenizeFormula(formula_str);

    // Iterate over each component and return SBML components with values associated
    for (int i = 0; i < components_vector.size(); i++) {
        formula_value_map[components_vector[i]] = this->component_map[components_vector[i]];
    
    }
    return formula_value_map;       
}

std::vector<std::string> StochasticModule::tokenizeFormula(const std::string& formula_str) {

    std::vector<std::string> tokens;

    std::string current_token_bin;

    for (char c : formula_str) {
        if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '(' || c == ')') {
            if (!current_token_bin.empty()) {
                tokens.push_back(current_token_bin);
            } 
            current_token_bin.clear();
        } else if (c != ' ') {
            current_token_bin += c;
        } else if (!current_token_bin.empty()) {
            tokens.push_back(current_token_bin);
            current_token_bin.clear();
        }
    }
    if (!current_token_bin.empty()) {
        tokens.push_back(current_token_bin);
    }
    return tokens;
}

bool StochasticModule::is_alnumus(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::string StochasticModule::safe_replace_alnumus(
    std::string &input,
    const std::string &swap,
    const std::string &with
) {
    if (swap.empty()) return input;

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

std::string StochasticModule::to_str(double val) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(15) << val;
    return out.str();
}

Eigen::VectorXd StochasticModule::samplePoisson(
    Eigen::VectorXd mu
) {

    // realization vector for storing random poisson samples
    Eigen::VectorXd m_i(mu.size()); 

    for (size_t i = 0; i < mu.size(); ++i) {

        std::poisson_distribution<int> dist((mu(i) * this->delta_t)); 
        m_i(i) = dist(this->generator);

    }
    return m_i;
}

double min_element_fast(const Eigen::VectorXd &vec) 

Eigen::VectorXd StochasticModule::constrainTau(
    Eigen::VectorXd m_i,
    Eigen::VectorXd xhat_tn
) {

    Eigen::VectorXd mhat_actual(m_i.size()); // results storage vector

    const int numCols = this->stoichmat.cols();
    for (int j = 0; j < numCols; ++j) {

        // Vector for curresnt ratelaw stoichiometries per species (i.e. column of S)
        const auto S_j = this->stoichmat.col(j);

        // calculate coefficient products of current state
        Eigen::ArrayXd Rhat_j = (xhat_tn.array() * S_j.array()).abs(); 

        // Compute min valid reactant or fallback to m_i(j)
        double R_mi = m_i(j);
        for (const double &val : Rhat_j) if (val > 0 && val < R_mi) R_mi = val;
        mhat_actual(j) = std::min(m_i(j), R_mi);
    }

    return mhat_actual;
}

Eigen::VectorXd StochasticModule::computeNewState(
    Eigen::VectorXd state_t,
    Eigen::VectorXd real_vec
) {

    // Update the stochastic state vector: new_state = old_state * v
    Eigen::VectorXd delta = stoichmat * real_vec;   // matrix-vector product
    Eigen::VectorXd new_state = (state_t + delta).array().round();  // elementwise rounding

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
    Eigen::VectorXd last_state_nM = this->getLastStepResult(step);  // nM

    // convert units to molecule per volume
    Eigen::VectorXd mpv_state = last_state_nM.array() * this->nM2mpv_conversion_factors.array();
    this->updateComponentMap(this->species_list, mpv_state);

    // Sample stochastic answer from Poisson distribution
    Eigen::VectorXd realizations = samplePoisson(computeReactions());

    // //reassign molecules per volume to just molecules:
    Eigen::VectorXd mol_state = this->getSpeciesValues().array() * this->species_volumes.array();

    // Constrain Tau-leap algorithm for conservation of moiety
    Eigen::VectorXd constrained_realizations = constrainTau(realizations,  mol_state);
    
    // Calculate the updated state for current step:
    Eigen::VectorXd new_state = computeNewState(mol_state, constrained_realizations);
    
    // convert units to nanoMolar
    Eigen::VectorXd nM_state = new_state.array() * this->molecules2nM_conversion_factors.array();

    // Convert map values back to nanomolar value
    this->updateComponentMap(this->species_list, nM_state);

    //Record iteration's result
    BaseModule::recordStepResult(nM_state, step);

}

void StochasticModule::run(
    Eigen::VectorXd timesteps
) {
    for (int t = 0; t < timesteps.size(); t++) {

        this->step(t);

    }
}
