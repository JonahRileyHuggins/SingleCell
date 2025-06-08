/**
 * @file: SingleCell.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Class Creator For Single Cell Model.
 */
//========================header file definition============================//
#pragma once

#ifndef SINGLECELL_h
#define SINGLECELL_h

//===========================Library Import=================================//

#include <vector>
#include <memory>
#include <optional>

//Internal Libraries
#include "sbml/SBMLReader.h"
#include "singlecell/BaseModule.h"
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"

//==========================Class Declaration===============================//
class SingleCell {
    private:

    protected:

    public:
    //---------------------------methods----------------------------------//
        
        SingleCell(
            const std::string& stochastic_sbml_path,
            const std::string& deterministic_sbml_path
        ); //Ctor

        virtual ~SingleCell() = default; //Dtor

        /**
         * @brief public method for users to interface with the SingleCell Simulator. 
         * 
         * @param entity_map map of species to be modified, with corresponding values
         * @param start is the simulation start time
         * @param stop is the simulation stop time, in seconds
         * @param step is the delta_t step between simulation updates in seconds
         * 
         * @returns matrix of global states for both models
         */
        std::vector<std::vector<double>> simulate(
            std::unordered_map<std::string, double> entity_map,
            double start = 0.0, //seconds
            double stop = 60.0, //seconds
            double step = 30.0 //seconds
        );

        /**
         * @brief getter method for retrieving all speciesIds from all associated submodels
         * uses each model's SBMLHandler->getSpeciesIds() method.
         * 
         * @param None non-static, uses class-members
         * 
         * @returns global_ids string vector of all species identifiers from all models
         * in parent class SingleCell
         */
        std::vector<std::string> getGlobalSpeciesIds();

        //---------------------------members--------------------------------//
        SBMLHandler StochasticModel;
        SBMLHandler DeterministicModel;
       

};

#endif // SINGLECELL_H
