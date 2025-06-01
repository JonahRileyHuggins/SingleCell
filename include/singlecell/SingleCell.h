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
#include "singlecell/Simulation.h"
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"

//==========================Class Declaration===============================//
class SingleCell {
    private:

    protected:

    public:
        SingleCell(
            const std::string& stochastic_sbml_path,
            const std::string& deterministic_sbml_path
        ); //Ctor

        virtual ~SingleCell() = default; //Dtor

        /**
         * @brief public method for users to interface with the SingleCell Simulator. 
         * 
         * @param det_states are the initial species values for the deterministic AMICI model
         * @param stoch_states are the initial species values for the stochastic SBML model
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

        // @TODO: need to change pointer to Model
        SBMLHandler StochasticModel;
        SBMLHandler DeterministicModel;
       

};

#endif // SINGLECELL_H
