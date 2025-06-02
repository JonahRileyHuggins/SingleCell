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
) : Simulation(StochasticModel) {

    // Retrieve the stoichiometric matrix from the sbml document.
    this->stoichmat = StochasticModel.getStoichiometricMatrix();

    matrix_utils::save_matrix(this->stoichmat, "../src/stoichmat.tsv");

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

        double R_mi = m_i[i]; // was set 0.0
        for (const auto& reactant : Rhat_i) {

            if (reactant < 0) { // drop reactants != negative (-): i.e. not rate-limiting
                R_mi = reactant;
            } 
        }
        double R_mi_u = std::abs(R_mi);

        // compare between predicted and actual:
        mhat_actual[i] = std::min(m_i[i], R_mi_u); 
    }


        printf( "Predicted v rates: \n");
        for (int j  = 0; j < m_i.size(); j++) {

        std::cout << m_i[j] << "\t";
        }
        printf("\n");

        printf("Assigned v rates: \n");
        for (int j  = 0; j < m_i.size(); j++) {

        std::cout << mhat_actual[j] << "\t";
        }
        printf("\n");



    return mhat_actual;
}

void StochasticModule::_simulationPrep(
    std::unordered_map<std::string, double>entity_map,
    double start, 
    double stop, 
    double step
) {

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

     int numSpecies = this->sbml->getNumSpecies();
     
     std::vector<double> timeSteps = Simulation::setTimeSteps(start, stop, step);

     this->results_matrix = Simulation::createResultsMatrix(numSpecies, timeSteps.size());
 
    Simulation::recordStepResult(
        init_states, 
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

    // Calculate v = formula where v is a vector of left hand reaction results
    std::vector<double> mu = computeReactions(last_record);

    // Sample stochastic answer from Poisson distribution
    std::vector<double> m_i = samplePoisson(mu);

    // Constrain Tau-leap algorithm to only positive integers:
    std::vector<double> mhat_actual = constrainTau(m_i, last_record);

    // Update the stochastic state vector: new_state = max((old_state * v), 0)
    std::vector<double> new_state(last_record.size());
    for (size_t i = 0; i < last_record.size(); ++i) {
        double delta = 0.0;
        for (size_t j = 0; j < mhat_actual.size(); ++j) {
            delta += stoichmat[i][j] * mhat_actual[j];
        }
        new_state[i] = last_record[i] + delta, 0.0;
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
