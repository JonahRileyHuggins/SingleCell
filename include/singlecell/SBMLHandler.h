/*
filename: SBMLHandler.h
created by: Jonah R. Huggins
created on: 25-05-09

description: 
Abstraction and Encapsulation class for manipulating the SBML document
*/
//--------------helper function definition------------------------------------//

#ifndef SBMLHandler_h
#define SBMLHandler_h

// -------------------------------Library Import-----------------------------//
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>
#include <sbml/SBMLTypes.h>
#include <sbml/SBMLReader.h>

//--------------------------Constants Declarations---------------------------//

//--------------------------Class Declaration-------------------------------//
class SBMLHandler {
public:
    SBMLHandler(const std::string& filename) { // Constructor method
        
        SBMLReader reader;
        doc = reader.readSBML(filename.c_str());
        if (!doc || doc->getNumErrors() > 0) {
            std::cerr << "Error Loading SBML File: " << filename << "\n";
            model = nullptr;
        
        } else {
            model = doc->getModel(); 
        }

    }

    ~SBMLHandler() { // Destructor Method
        delete doc; 
        delete model;
    }

    Model* getModel() const { // Provides direct access to the loaded model.
        return model;
    }

    std::vector<std::vector<double>> getStoichiometricMatrix() {
        /* The stoichiometric matrix is a N x M matrix composed of N-number of species
        by M-number of reactions. 
        */

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
                const SpeciesReference* reactant = reactants->get(r);
                const std::string speciesId = reactant->getSpecies();

                double coeff = reactant->getStoichiometry();

                unsigned int speciesIndex = species_map.at(speciesId);

                stoichmat[speciesIndex][i] -= coeff;
            }

            const ListOfSpeciesReferences* products = reaction->getListOfProducts();

            // Find Products
            for (unsigned int p = 0; p < products->size(); p++) {
                const SpeciesReference* product = products->get(p);
                const std::string speciesId = product->getSpecies();

                double coeff = product->getStoichiometry();

                unsigned int speciesIndex = species_map.at(speciesId);

                stoichmat[speciesIndex][i] += coeff;
            }
        }

        return stoichmat;
    }

    std::unordered_map<std::string, unsigned int> speciesMap(const int& numSpecies) {

        std::unordered_map<std::string, unsigned int> speciesIndexMap;

        Model* sbml_model = this->model; 

        for (unsigned int i = 0; i < numSpecies; ++i) {

            speciesIndexMap[sbml_model->getSpecies(i)->getId()] = i;
        }

        return speciesIndexMap;
    }

    std::vector<std::string> getReactionExpressions() {
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

            const RateLaw* ratelaw = reaction->getRateLaw();

            if (ratelaw != nullptr) {
            const ASTNode* ast_node = ratelaw->getMath();

            formulas_vector[i] = SBML_formulaToL3String(ast_node);
            } else {
                formulas_vector[i] = nullptr;
            }
        }
        return formulas_vector;
    }

    std::vector<std::string> getSpeciesIds() {
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

    private:
        SBMLDocument* doc = nullptr; 
        Model* model = nullptr; 
};

#endif