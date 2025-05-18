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

        Model* getModel(); // Provides direct access to the loaded model.

        std::vector<std::vector<double>> getStoichiometricMatrix();

        std::unordered_map<std::string, unsigned int> speciesMap(
            const int& numSpecies
        );

        std::vector<std::string> getReactionExpressions();

        std::vector<std::string> getSpeciesIds();

        std::vector<double> getInitialState();

    private:
        SBMLDocument* doc = nullptr; 
        Model* model = nullptr; 

};

#endif