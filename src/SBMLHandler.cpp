/**
 * @file SBMLHandler.cpp
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 08-05-2025
 * 
 * @brief Abstraction and Encapsulation class for manipulating the SBML document
 */
// -------------------------------Library Import-----------------------------//
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <sbml/SBMLTypes.h>
#include <sbml/SBMLReader.h>

// Internal Libraries
#include "singlecell/SBMLHandler.h"
//--------------------------Constants Declarations---------------------------//

//--------------------------Class Declaration-------------------------------//
SBMLHandler::SBMLHandler(const std::string& filename) { // Constructor method
    /**
     *  @brief Class instance call
     * 
     * @param filename path to SBML Model file
     * 
     * @returns None
     */    
    SBMLReader reader;
    doc = reader.readSBML(filename.c_str());
    if (!doc || doc->getNumErrors() > 0) {
        std::cerr << "Error Loading SBML File: " << filename << "\n";
        model = nullptr;
    
    } else {
        model = doc->getModel(); 
    }

}

SBMLHandler::~SBMLHandler() { // Destructor Method
    delete doc; 
    delete model;
}

Model* SBMLHandler::getModel() { // Provides direct access to the loaded model.
    return model;
}

std::vector<std::vector<double>> SBMLHandler::getStoichiometricMatrix() {
    /**
     * @brief The stoichiometric matrix is a N x M matrix composed of N-number of species
     * by M-number of reactions. 
     * 
     * @param None
     * 
     * @returns stoichmat A stochiometric matrix 
     * */

    Model* sbml_model = this->model; 

    unsigned int numSpecies = sbml_model->getNumSpecies();

    unsigned int numReactions = sbml_model->getNumReactions();

    // make map of species indices
    std::unordered_map<std::string, unsigned int> species_map = speciesMap(numSpecies);

    // build a blank stoichiometric matrix of zeros
    std::vector<std::vector<double>> stoichmat(numSpecies, std::vector<double>(numReactions, 0.0));

    // Populate the matrix:
    for (unsigned int i = 0; i < numReactions; i++) {
        //Reaction getter
        const Reaction* reaction = sbml_model->getReaction(i);
        
        // Find Reactants
        const ListOfSpeciesReferences* reactants = reaction->getListOfReactants();

        for (unsigned int r = 0; r < reactants->size(); r++) {
            const SimpleSpeciesReference* reactant_ref = reactants->get(r);
            const SpeciesReference* reactant = dynamic_cast<const SpeciesReference*>(reactant_ref);
            const std::string speciesId = reactant->getSpecies();
            double coeff = reactant->getStoichiometry();
            unsigned int speciesIndex = species_map.at(speciesId);
            stoichmat[speciesIndex][i] -= coeff;
        }

        const ListOfSpeciesReferences* products = reaction->getListOfProducts();

        // Find Products
        for (unsigned int p = 0; p < products->size(); p++) {
            const SimpleSpeciesReference* product_ref = products->get(p);
            const SpeciesReference* product = dynamic_cast<const SpeciesReference*>(product_ref);
            const std::string speciesId = product->getSpecies();
            double coeff = product->getStoichiometry();
            unsigned int speciesIndex = species_map.at(speciesId);
            stoichmat[speciesIndex][i] += coeff;
        }
    }

    return stoichmat;
}

std::unordered_map<std::string, unsigned int> SBMLHandler::speciesMap(const int& numSpecies) {
    /**
     * @brief creates a map of species identifiers to thier corresponding index
     * 
     * @param numSpecies integer count of species in the SBML model
     * 
     * @returns speciesIndexMap map of species identifiers and the corresponding index
     */
    std::unordered_map<std::string, unsigned int> speciesIndexMap;

    Model* sbml_model = this->model; 

    for (unsigned int i = 0; i < numSpecies; ++i) {

        speciesIndexMap[sbml_model->getSpecies(i)->getId()] = i;
    }

    return speciesIndexMap;
}

std::vector<std::string> SBMLHandler::getReactionExpressions() {
    /** 
     * @brief Gets vector of formulas as strings
     * 
     * @param None
     * 
     * @returns formulas_vector: a vector of reaction formulas in string format.
    */
    Model* sbml_model = this->model;

    unsigned int numReactions = sbml_model->getNumReactions();

    double v_i(numReactions);

    // create a list to return:
    std::vector<std::string> formulas_vector(v_i);

    // Populate the matrix:
    for (unsigned int i = 0; i < numReactions; i++) {
        //Reaction getter
        const Reaction* reaction = sbml_model->getReaction(i);

        const KineticLaw* ratelaw = reaction->getKineticLaw();

        if (ratelaw != nullptr) {
        const ASTNode* ast_node = ratelaw->getMath();

        formulas_vector[i] = SBML_formulaToL3String(ast_node);
        } else {
            formulas_vector[i] = nullptr;
        }
    }
    return formulas_vector;
}

std::vector<std::string> SBMLHandler::getSpeciesIds() {
    /**
     * @brief getter method for returning all model ids as a vector
     * 
     * @param None
     * 
     * @returns species_ids vector of species identifiers in SBML model
     */

    Model* model = this->model;

    unsigned int num_species = model->getNumSpecies();

    std::vector<std::string> species_ids(num_species);

    for (int i = 0; i < num_species; i++) {

        species_ids[i] = model->getSpecies(i)->getId();

    }
    return species_ids;
}

std::vector<double> SBMLHandler::getInitialState() {
    /**
     * @brief getter method for obtaining SBML intial state values
     * 
     * @param None
     * 
     * @returns initial_state vector of double initial model states for every species
     */
     int numSpecies = SBMLHandler::getModel()->getNumSpecies();

     std::vector<double> initial_state(numSpecies);

     for (unsigned int i = 0; i < numSpecies; i++) {
        double state = SBMLHandler::getModel()->getSpecies(i)->getInitialConcentration();
        
        initial_state[i] = state;
     }
}

std::vector<std::string> SBMLHandler::getParameterIds() {
    std::vector<std::string> parameter_ids;
    Model* model = this->model;

    if (!model) return parameter_ids;

    unsigned int numParams = model->getNumParameters();
    for (unsigned int i = 0; i < numParams; ++i) {
        const Parameter* param = model->getParameter(i);
        if (param) {
            parameter_ids.push_back(param->getId());
        }
    }

    return parameter_ids;
}
