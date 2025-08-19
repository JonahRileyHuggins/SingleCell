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
#include "singlecell/StochasticModule.h"

// Third Party Libraries
#include "muParser.h"

//=============================Class Details================================//
StochasticModule::StochasticModule(
    SBMLHandler StochasticModel
) : BaseModule(StochasticModel) {

    // Retrieve the stoichiometric matrix from the sbml document.
    this->stoichmat = StochasticModel.getStoichiometricMatrix();

    // List of formula strings to be parsed.
    this->formulas_vector = StochasticModel.getReactionExpressions();

    //Instantiate SBML model
    this->sbml = StochasticModel.model;

    //call conversion method here:
    this->nM2mpv_conversion_factors = unit_conversions::nanomolar2mpv(StochasticModel.species_volumes);
    this->molecules2nM_conversion_factors = unit_conversions::molecules2nanomolar(StochasticModel.species_volumes);


    this->algorithm_id = this->sbml->getId();
    this->target_id = "deterministic";

 }

std::string StochasticModule::getModuleId() { return this->algorithm_id; }

void StochasticModule::loadTargetModule(
    const std::vector<std::unique_ptr<BaseModule>>& module_list
) {
    for (const auto& mod : module_list) {

        if (mod->getModuleId() == this->target_id) {

            this->targets.push_back(mod.get());
        }

    }
}

std::vector<double> StochasticModule::computeReactions(const std::vector<double>& state) {
    /** 
     * @brief Computes all reactions in the SBML model
     * 
     * @param state the initial states of all species in the SBML model
     * 
     * @returns v vector of state values after initial stochiometric calculations
    */
    
    unsigned int numReactions = sbml->getNumReactions();

    std::vector<double> v(numReactions);

    // Populate the matrix:
    for (unsigned int i = 0; i < numReactions; i++) {
        //Reaction getter
        std::string formula_i = formulas_vector[i];

        v[i] = computeReaction(formula_i);
    }
    
    return v;
}
    
double StochasticModule::computeReaction(std::string formula_str) {

    // Create instance of Parser object
    mu::Parser parser;

    // get variables in formula
    std::unordered_map<std::string, double> components = mapComponentsToValues(formula_str);
    //Persistent copy of component values:
    std::unordered_map<std::string, double> component_values;

    try {
    for (const auto& [name, value] : components) {
        component_values[name] = value;
        parser.DefineVar(name, &component_values[name]);
    }
    parser.SetExpr(formula_str);

    double v_i = parser.Eval();

    return v_i;

    }
    catch (mu::Parser::exception_type &e) {
        std::cout << "Error while parsing: " << e.GetMsg() << "\n";
        return std::numeric_limits<double>::quiet_NaN();
    }
}

std::unordered_map<std::string,double> StochasticModule::mapComponentsToValues(const std::string& formula_str) {

    std::unordered_map<std::string, double> component_value_map;

    std::vector<std::string> components_vector = tokenizeFormula(formula_str);

    // Iterate over each component and return SBML components with values associated
    for ( int i = 0; i < components_vector.size(); i++) {

        const std::string component = components_vector[i];

        // Check if in SBML as Parameter || Species || Compartment;
        if (sbml->getParameter(component)!= nullptr) {
            double value = sbml->getParameter(component)->getValue();
            component_value_map[component] = value;
        } else if (sbml->getSpecies(component) != nullptr) {
            double value = sbml->getSpecies(component)->getInitialConcentration();
            component_value_map[component] = value;
        } else if (sbml->getCompartment(component)!= nullptr) {
            double value = sbml->getCompartment(component)->getVolume();
            component_value_map[component] = value;
        } 
    }

    return component_value_map;
        
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

        std::vector<double> abs_r;
        abs_r.reserve(Rhat_i.size());

        for (const auto& rct : Rhat_i) {
            double abs_val = std::abs(rct);
            if (abs_val > 0)
                abs_r.push_back(abs_val);
        }

        double R_mi = m_i[i]; // was set 0.0
        for (const auto& reactant : abs_r) {

            if (reactant < R_mi) { // drop reactants != negative (-): i.e. not rate-limiting
                R_mi = reactant;
            }
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

    int numSpecies = this->sbml->getNumSpecies();

    this->timesteps = BaseModule::setTimeSteps(start, stop, step);

    this->results_matrix = BaseModule::createResultsMatrix(numSpecies, timesteps.size());

    BaseModule::recordStepResult(
        this->handler.getInitialState(), 
        0
    );

}

void StochasticModule::setModelState(const std::vector<double>& state) {

    std::vector<std::string> speciesIds = handler.getSpeciesIds();
    for (size_t i = 0; i < speciesIds.size(); ++i) {
        Species* s = sbml->getSpecies(speciesIds[i]);
        s->setInitialConcentration(state[i]);
    }
}

void StochasticModule::step(
    int step
) {
    // get (step minus 1) position in results_matrix member
    std::vector<double> last_state_nM = getLastStepResult(step);  // nM

    //reset SBML species values:
    this->handler.setState(last_state_nM); // nM 

    //convert from nanomolar to mpc:
    this->handler.convertSpeciesUnits(this->nM2mpv_conversion_factors); // molecules per volume

    // Sample stochastic answer from Poisson distribution
    std::vector<double> realizations = samplePoisson(computeReactions(this->handler.getInitialState()));

    //reassign molecules per volume to just molecules:
    this->handler.convertSpeciesUnits(this->handler.species_volumes);

    // Constrain Tau-leap algorithm for conservation of mass
    std::vector<double> constrained_realizations = constrainTau(
        realizations, 
        this->handler.getInitialState()
    );
    
    // Calculate the updated state for current step:
    std::vector<double> new_state = computeNewState(
        this->handler.getInitialState(),
        constrained_realizations
    );

    this->handler.setState(new_state);

    // Convert back into nanomolar
    this->handler.convertSpeciesUnits(this->molecules2nM_conversion_factors);

    //Record iteration's result
    BaseModule::recordStepResult(this->handler.getInitialState(), step);

}

void StochasticModule::run(
    std::vector<double> timesteps
) {
    for (int t = 0; t < timesteps.size(); t++) {

        this->step(t);

    }
}

void StochasticModule::updateParameters() {

    for (const auto& alt_module : this->targets) {

        SBMLHandler alternate_model = alt_module->handler;

        std::vector<std::string> species_list = alternate_model.getSpeciesIds();

        for (int i = 0; i < this->overlapping_params.size(); ++i) {
            const std::string& id = this->overlapping_params[i];

            Parameter* parameter = this->sbml->getParameter(id);

            Species* species = alternate_model.model->getSpecies(id);

            if (species == nullptr) {
                std::cerr << "[Warning] Species with ID '" << id << "' not found in alternate model.\n";
                continue; // skip to next iteration
            }

            if (parameter == nullptr) {
                std::cerr << "[Warning] Parameter with ID '" << id << "' not found in sbml.\n";
                continue;
            }

            parameter->setValue(species->getInitialConcentration());
        }

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
