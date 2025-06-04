/**
 * @file SBMLHandler.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 08-05-2025
 * 
 * @brief Abstraction and Encapsulation class for manipulating the SBML document
 */
//--------------helper function definition------------------------------------//
#pragma once

#ifndef SBMLHandler_h
#define SBMLHandler_h

// -------------------------------Library Import-----------------------------//
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <sbml/SBMLTypes.h>
#include <sbml/SBMLReader.h>
//--------------------------Constants Declarations---------------------------//

//--------------------------Class Declaration-------------------------------//
class SBMLHandler {
    public:
        SBMLHandler( // Constructor method
            const std::string& filename
        ); 

        ~SBMLHandler(); // Destructor Method

        Model* model; 

        /**
        * @brief The stoichiometric matrix is a N x M matrix composed of N-number of species
        * by M-number of reactions. 
        * 
        * @param None
        * 
        * @returns stoichmat A stochiometric matrix 
        * */
        std::vector<std::vector<double>> getStoichiometricMatrix();

        /**
         * @brief creates a map of species identifiers to thier corresponding index
         * 
         * @param numSpecies integer count of species in the SBML model
         * 
         * @returns speciesIndexMap map of species identifiers and the corresponding index
         */
        std::unordered_map<std::string, unsigned int> speciesMap(
            const int& numSpecies
        );

        /** 
         * @brief Gets vector of formulas as strings
         * 
         * @param None
         * 
         * @returns formulas_vector: a vector of reaction formulas in string format.
        */
        std::vector<std::string> getReactionExpressions();

        /**
         * @brief getter method for returning all model ids as a vector
         * 
         * @param None
         * 
         * @returns species_ids vector of species identifiers in SBML model
         */
        std::vector<std::string> getSpeciesIds();

        /**
         * @brief getter method for obtaining SBML intial state values
         * 
         * @param None
         * 
         * @returns initial_state vector of double initial model states for every species
         */
        std::vector<double> getInitialState();

        /**
         * @todo add method docstring
         */
        std::vector<std::string> getParameterIds();

        /**
         * @brief Method for modifying model entity species || parameter|| compartment
         * 
         * @param entity_id model entity string or parameter to index for change
         * @param new_value updating value for model
         * 
         * @returns None updates model SBML object
         */
        void setModelEntityValue(
        std::string entity_id, 
        double new_value
        );

        /**
         * @brief gets list of reactionId strings
         * 
         * @param None
         * 
         * @returns reactionIds list of reaction identifiers
         */
        std::vector<std::string> getReactionIds();

        /**
         * @brief getter method for retrieving species-specific compartmental volumes
         * 
         * @param None
         * 
         * @returns cell_volumes vector list of compartmental volumes, as defined in
         * class member this->model
         */
        std::vector<double> getGlobalSpeciesCompartmentVals();

        /**
         * @brief add method docstring
         */
        void convertSpeciesUnits(
            std::vector<double> conversion_factor
        );

        /**
         * @brief assigns new state to SBML model
         * 
         * @param new_state list of new values, 
         * length must match number of species in model
         * 
         * @returns None updates SBML model species globally
         */
        void setState(
            std::vector<double> new_state
        );

    //----------------------------members-----------------------------------//
        std::vector<double> species_volumes;


    private:
    //---------------------------------methods------------------------------//

    //-------------------------------members--------------------------------//
        SBMLDocument* doc; 

};

#endif