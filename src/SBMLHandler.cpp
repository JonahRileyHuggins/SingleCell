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

//--------------------------Class Declaration-------------------------------//
SBMLHandler::SBMLHandler(
    const std::string& filename
) { // Constructor method
    /**
     *  @brief Class instance call
     * 
     * @param filename path to SBML Model file
     * 
     * @returns None
     */    
    SBMLReader reader;
    doc = reader.readSBML(filename.c_str());

    model = doc->getModel(); 
    }

SBMLHandler::~SBMLHandler() { // Destructor Method
    if (doc != nullptr) {
        doc = nullptr;
        delete doc;
    }

    if (model != nullptr) {
        model = nullptr;
        delete model;
    }
}

std::vector<std::vector<double>> SBMLHandler::getStoichiometricMatrix() {

    int numSpecies = this->model->getNumSpecies();

    int numReactions = this->model->getNumReactions();

    // make map of species indices
    std::unordered_map<std::string, unsigned int> species_map = speciesMap(numSpecies);

    // build a blank stoichiometric matrix of zeros
    std::vector<std::vector<double>> stoichmat(numSpecies, std::vector<double>(numReactions, 0.0));

    // Populate the matrix:
    for (int i = 0; i < numReactions; i++) {
        //Reaction getter
        const Reaction* reaction = this->model->getReaction(i);
        
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

    std::unordered_map<std::string, unsigned int> speciesIndexMap;

    for (unsigned int i = 0; i < numSpecies; ++i) {

        speciesIndexMap[this->model->getSpecies(i)->getId()] = i;
    }

    return speciesIndexMap;
}

std::vector<std::string> SBMLHandler::getReactionExpressions() {

    unsigned int numReactions = this->model->getNumReactions();

    double v_i(numReactions);

    // create a list to return:
    std::vector<std::string> formulas_vector(v_i);

    // Populate the matrix:
    for (unsigned int i = 0; i < numReactions; i++) {
        //Reaction getter
        const Reaction* reaction = this->model->getReaction(i);

        const KineticLaw* ratelaw = reaction->getKineticLaw();

        const ASTNode* ast_node = ratelaw->getMath();

        formulas_vector[i] = SBML_formulaToL3String(ast_node);

    }
    
    return formulas_vector;
}

std::vector<std::string> SBMLHandler::getSpeciesIds() {

    unsigned int num_species = this->model->getNumSpecies();

    std::vector<std::string> species_ids(num_species);

    for (int i = 0; i < num_species; i++) {

        species_ids[i] = model->getSpecies(i)->getId();

    }
    return species_ids;
}

std::vector<double> SBMLHandler::getInitialState() {

     int numSpecies = this->model->getNumSpecies();

     std::vector<double> initial_state(numSpecies);

     for (unsigned int i = 0; i < numSpecies; i++) {

        double state = this->model->getSpecies(i)->getInitialConcentration();
        
        initial_state[i] = state;
     }

     return initial_state;
}

std::vector<std::string> SBMLHandler::getParameterIds() {
    std::vector<std::string> parameter_ids;

    int numParams = this->model->getNumParameters();
    for (int i = 0; i < numParams; ++i) {
        const Parameter* param = this->model->getParameter(i);

            parameter_ids.push_back(param->getId());
    }

    return parameter_ids;
}
