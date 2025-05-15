/**
 * @file stochastic_module.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Definitions for StochasticModule operations
*/

// --------------------------Library Import-----------------------------------//

#include <ctime>
#include <vector>
#include <string>
#include <random>
#include <memory>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>

// Internal libraries
#include "singlecell/SBMLHandler.h"
#include "singlecell/StochasticModule.h"

// Third Party Libraries
#include "muParser.h"

//-----------------------------Method Details-----------------------------//
//Constructor: initialize sbmlHandler with a new SBMLHandler instance
StochasticModule::StochasticModule(
    const std::string& sbml_path, 
    double timeStep
) : sbmlHandler(std::make_unique<SBMLHandler>(sbml_path)),
    step(timeStep)
 {
    // Retrieve the stoichiometric matrix from the sbml document.
    stoichmat = sbmlHandler->getStoichiometricMatrix();

    // List of formula strings to be parsed. <-- !!! Might swap for something ASTNode compatible later.
    formulas_vector = sbmlHandler->getReactionExpressions();

 }

std::vector<double> StochasticModule::computeReactions(const std::vector<double>& state) {
    /** 
     * @brief Computes all reactions in the SBML model
     * 
     * @param state the initial states of all species in the SBML model
     * 
     * @returns v vector of state values after initial stochiometric calculations 
    */
    
    Model* sbml_model = sbmlHandler.model;

    unsigned int numReactions = sbml_model->getNumReactions();

    std::vector<double> v(numReactions);

    // Populate the matrix:
    for (unsigned int i = 0; i < numReactions; i++) {
        //Reaction getter
        std::vector<std::string> formula_i = formulas_vector[i];

        v[i] = computeReaction(formula_i, state);
    }
    
    return v;
}
    
double StochasticModule::computeReaction(std::string formula_str) {
    /**
    * @brief class instance to calculate a reaction formula using the muParser object
    * 
    * @param formula_str reaction formula to be calculated
    * 
    * @return v_i reaction i's left hand result
    */

    // Create instance of Parser object
    mu::Parser parser;

    // get variables in formula
    std::unordered_map<std::string, double> components = StochasticModule::mapComponentsToValues(formula_str);
    try {
    for (const auto& entity : components) {

        parser.DefineVar(entity.first, entity.second);
    }
    parser.SetExpr(formula_str);

    double v_i = parser.Eval();

    return v_i;

    }
    catch (mu::Parser::exception_type &e) {
        std::cout <<< e.GetMsg() <<< "\n";
    }
}

std::unordered_map<std::string,double> StochasticModule::mapComponentsToValues(const std::string& formula_str) {
    /**
     * @brief Finds all species in the formula string 
     *      NOTE: species must be initialConcentration only. 
     *      TODO: make compatible with Species initialAmount
     * 
     * @param formula_str string form of the reaction formula
     * 
     * @returns Map of component IDs to their numerical value
     */

    std::unordered_map<std::string, double> component_value_map;

    Model* sbml_model = sbmlHandler.model;

    std::vector<std::string> components_vector = tokenizeFormula(formula_str);

    // Iterate over each component and return SBML components with values associated
    for (const auto& i : components_vector) {

        const std::string component = components_vector[i];

        // Check if in SBML as Parameter || Species || Compartment;
        if (sbml_model->getParameter(component)!= nullptr) {
            double value = sbml_model->getParameter(component)->getValue();
            component_value_map[component] = value;
        } else if (sbml_model->getSpecies(component) != nullptr)) {
            double value = sbml_model->getSpecies(component)->getValue();
            component_value_map[component] = value;
        } else if (sbml_model->getCompartment(component)!= nullptr) {
            double value = sbml_model->getCompartment(component)->getVolume();
            component_value_map[component] = value;
        } 
    }

    return component_value_map;
        
}

std::vector<std::string> StochasticModule::tokenizeFormula(const std::string& formula_str) {
    /**
     * @brief Creates a list of strings based on formula contents
     * 
     * @param formula_str string type reaction formula
     * 
     * @returns tokens a list of components from the formula, without operators
     */
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

std::vector<double> StochasticModule::samplePoisson(std::vector<double> initial_reaction_vector) {
    /** 
     * @brief Update stoichiometric values by setting as the mean for a poission distribution
     * 
     * @param initial_reaction_vector is the calculated result of computeReactions methodacosf128
     * 
     * @returns stochastic_array vector of Poisson-dist updated values
    */

    // Global Mersenne Twister generator seeded with current time
    static std::mt19937 mrand(static_cast<unsigned int>(std::time(nullptr)));

    std::vector<double> stochastic_array(initial_reaction_vector.size()); 

    for (size_t i = 0; i < initial_reaction_vector.size(); ++i) { // NEEDS WORK HERE!

        std::poisson_distribution<int> d((initial_reaction_vector[i] * this.step));

        stochastic_array[i] = d(mrand);
    }

    return stochastic_array;

}



void StochasticModule::setModelState(const std::vector<double>& state) {
    /**
     * @brief public method for updating the simulation states at every timestep. 
     *      TODO: Still need to assign updated parameter values from the deterministic model. 
     * 
     * @param state vector of timestep values to be calculated. 
     * 
     * @returns None
     */
    auto speciesIds = sbmlHandler->getSpeciesIds();
    for (size_t i = 0; i < speciesIds.size(); ++i) {
        Species* s = sbmlHandler.model->getSpecies(speciesIds[i]);
        s->setInitialConcentration(state[i]);
    }
}

std::vector<double> StochasticModule::runStochasticStep(const std::vector<double>& stochastic_states) {
    /**
     * @brief Calculates a single timestep for the stochastic module
     * 
     * @param stochastic_state The current state vector of the sbml model.
     * 
     * @returns new_state t+1 values for stochastic step.
    */

    // Calculate v = formula where v is a vector of left hand reaction values
    std::vector<double> v = computeReactions(stochastic_states);

    // Sample stochastic answer from Poisson distribution
    std::vector<double> r = samplePoisson(v);

    // Update the stochastic state vector: new_state = max((old_state * ))
    std::vector<double> new_state(stochastic_states.size());
    for (size_t i = 0; i < stochastic_states.size(); ++i) {
        double delta = 0.0;
        for (size_t j = 0; j < r.size(); ++j) {
            delta += stoichmat[i][j] * r[j];
        }
        new_state[i] = std::max(stochastic_states[i] + delta, 0.0);
    }

    return new_state;
}

}
