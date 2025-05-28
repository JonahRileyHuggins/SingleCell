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
#include <stdio.h>
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
#include "singlecell/StochasticModule.h"

// Third Party Libraries
#include "muParser.h"

//=============================Class Details================================//
StochasticModule::StochasticModule(
    SBMLHandler StochasticModel
) : Simulation(StochasticModel) {

    // Retrieve the stoichiometric matrix from the sbml document.
    this->stoichmat = StochasticModel.getStoichiometricMatrix();

    // List of formula strings to be parsed. 
    this->formulas_vector = StochasticModel.getReactionExpressions();

    //Instantiate SBML model
    this->sbml = StochasticModel.model;

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
            // tokens.push_back(std::string(1, c));
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

std::vector<int> StochasticModule::samplePoisson(
    std::vector<double> initial_reaction_vector
) {

    std::random_device rd;
    std::mt19937 generator(rd());

    std::vector<int> stochastic_array(initial_reaction_vector.size()); 

    for (size_t i = 0; i < initial_reaction_vector.size(); ++i) {

        std::poisson_distribution<int> dist((initial_reaction_vector[i] * this->delta_t)); 
        stochastic_array[i] = dist(generator);

    }
    return stochastic_array;
}

void StochasticModule::_simulationPrep(
    const std::vector<double>& initial_state,
    double start, 
    double stop, 
    double step
) {

    std::vector<double> stoch_states;

    if (initial_state.size() > 0) {
        stoch_states = initial_state;  
    } else {
        stoch_states = handler.getInitialState();
    }

     int numSpecies = this->sbml->getNumSpecies();
     
     std::vector<double> timeSteps = Simulation::setTimeSteps(start, stop, step);

     this->results_matrix = Simulation::createResultsMatrix(numSpecies, timeSteps.size());
 
    Simulation::recordStepResult(
        stoch_states, 
        0
    );

    this->delta_t = step;
}

void StochasticModule::setModelState(const std::vector<double>& state) {

    std::vector<std::string> speciesIds = handler.getSpeciesIds();
    for (size_t i = 0; i < speciesIds.size(); ++i) {
        Species* s = sbml->getSpecies(speciesIds[i]);
        s->setInitialConcentration(state[i]);
    }
}

void StochasticModule::runStep(
    int step
) {
    // get (step minus 1) position in results_matrix member
    std::vector<double> last_record = getLastStepResult(step);

    // Calculate v = formula where v is a vector of left hand reaction values
    std::vector<double> v = computeReactions(last_record);

    // Sample stochastic answer from Poisson distribution
    std::vector<int> r = samplePoisson(v);

    // Update the stochastic state vector: new_state = max((old_state * ))
    std::vector<double> new_state(last_record.size());
    for (size_t i = 0; i < last_record.size(); ++i) {
        double delta = 0.0;
        for (size_t j = 0; j < r.size(); ++j) {
            delta += stoichmat[i][j] * r[j];
        }
        new_state[i] = std::max(last_record[i] + delta, 0.0);
    }
    
    //Record iteration's result
    Simulation::recordStepResult(new_state, step);
}

void StochasticModule::updateParameters(
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
        
        Parameter* parameter = sbml->getParameter(overlapping_params[i]);

        parameter->setValue(alternate_model->getSpecies(overlapping_params[i])->getInitialConcentration());
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
