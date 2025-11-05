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
#include "utils.h"
#include "SBMLHandler.h"
#include "StochasticModule.h"

// external library
#include "parser.h"

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
    this->store = this->params_list;

 }

std::string StochasticModule::getModuleId() { return this->algorithm_id; }

std::vector<double> StochasticModule::computeReactions() {
    /** 
     * @brief Computes all reactions in the SBML model
     * 
     * @returns v vector of state values after initial stochiometric calculations
    */
    
    unsigned int numReactions = this->formulas_vector.size();

    std::vector<double> v(numReactions);

    // Populate the matrix:
    for (unsigned int i = 0; i < numReactions; i++) {
        
        std::string formula_i = formulas_vector[i];

        v[i] = computeReaction(formula_i);
    }
    
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
        double v_i = parser(new_formula_str.c_str());
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
        std::string component = components_vector[i];
        formula_value_map[component] = this->component_map[component];
    
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

std::vector<double> StochasticModule::samplePoisson(
    std::vector<double> mu
) {

    std::random_device rd;
    std::mt19937 generator(rd());

    // realization vector for storing random poisson samples
    std::vector<double> m_i(mu.size()); 

    for (size_t i = 0; i < mu.size(); ++i) {

        std::poisson_distribution<int> dist((mu[i] * this->delta_t)); 
        m_i[i] = dist(generator);

    }
    return m_i;
}

std::vector<double> StochasticModule::constrainTau(
    std::vector<double> m_i,
    std::vector<double> xhat_tn
) {

    std::vector<double> mhat_actual(m_i.size()); // results storage vector

    for (int i = 0; i < this->stoichmat[0].size(); i++) {

        // Vector for current ratelaw stoichiometries per species (i.e. column of S)
        std::vector<double> S_i = matrix_utils::getColumn(this->stoichmat, i);

        std::vector<double> Rhat_i(xhat_tn.size()); // double for storing each reaction product

        for (int j = 0; j < xhat_tn.size(); j++) {
            Rhat_i[j] = xhat_tn[j] * S_i[j]; // calculate coefficient products of current state
        }

        // <-- Modify starting here: check as source for possible error
        // retrieve all consumed reactants
        std::vector<double> abs_r(Rhat_i.size());
        size_t count = 0;
        for (size_t r = 0; r < Rhat_i.size(); ++r) {
            double abs_val = std::abs(Rhat_i[r]);
            if (abs_val > 0)
                abs_r[count++] = abs_val;
        }
        abs_r.resize(count); // trim unused entries

        double R_mi = m_i[i];
        for (double reactant : abs_r) {
            if (reactant < R_mi) // drop reactants != negative (-): i.e. not rate-limiting
                R_mi = reactant;
        }

        mhat_actual[i] = R_mi;
    }

    return mhat_actual;
}

std::vector<double> StochasticModule::computeNewState(
    std::vector<double> state_t,
    std::vector<double> real_vec
) {

        // Update the stochastic state vector: new_state = old_state * v
    std::vector<double> new_state(state_t.size());
    
    for (size_t i = 0; i < state_t.size(); ++i) {
        double delta = 0.0;
        for (size_t j = 0; j < real_vec.size(); ++j) {
            delta += stoichmat[i][j] * real_vec[j];
        }

        new_state[i] = std::round(state_t[i] + delta);
    }

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

void StochasticModule::setModelState(const std::vector<double>& state) {

    for (size_t i = 0; i < this->species_list.size(); ++i) {

        this->component_map[this->species_list[i]] = state[i];

    }
}

void StochasticModule::step(
    int step
) {
    // get (step minus 1) position in results_matrix member
    std::vector<double> last_state_nM = this->getLastStepResult(step);  // nM

    // convert units to molecule per volume
    std::vector<double> mpv_state(last_state_nM);
    for (int i = 0; i < mpv_state.size(); i++) {
        mpv_state[i] = last_state_nM[i] * this->nM2mpv_conversion_factors[i];
    }
    this->updateComponentMap(
        this->species_list,
        mpv_state
    );

    // Sample stochastic answer from Poisson distribution
    std::vector<double> realizations = samplePoisson(computeReactions());

    // //reassign molecules per volume to just molecules:
    std::vector<double> mol_state(mpv_state.size());
    std::vector<double> saved_state = this->getSpeciesValues();
    for (int i = 0; i < mol_state.size(); i++) {
        mol_state[i] = saved_state[i] * this->species_volumes[i];
    }

    // Constrain Tau-leap algorithm for conservation of moiety
    std::vector<double> constrained_realizations = constrainTau(
        realizations, 
        mol_state
    );
    
    // Calculate the updated state for current step:
    std::vector<double> new_state = computeNewState(
        mol_state,
        constrained_realizations
    );
    
    // convert units to nanoMolar
    std::vector<double> nM_state(new_state);
    for (int i = 0; i < nM_state.size(); i++) {
        nM_state[i] = new_state[i] * this->molecules2nM_conversion_factors[i];
    }

    // Convert map values back to nanomolar value
    this->updateComponentMap(
        this->species_list, // Variables to be converted
        nM_state // nM-converted results
    );

    //Record iteration's result
    BaseModule::recordStepResult(nM_state, step);

}

void StochasticModule::run(
    std::vector<double> timesteps
) {
    for (int t = 0; t < timesteps.size(); t++) {

        this->step(t);

    }
}

std::vector<double> StochasticModule::getLastStepResult(
    int timestep
) {

    std::vector<double> state_vector(this->results_matrix.size());

    state_vector = this->results_matrix[
        (timestep > 0) ? timestep - 1 : timestep
    ];

    return state_vector;
}
