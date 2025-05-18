/**
 * @file: SingleCell.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Class Creator For Single Cell Model.
 */
//----------------------header file definition-----------------------------//
#pragma once

#ifndef SINGLECELL_h
#define SINGLECELL_h

// --------------------------Library Import--------------------------------//

#include <vector>
#include <memory>
#include <optional>

//Internal Libraries
#include "sbml/SBMLReader.h"
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"

//--------------------------Class Declaration-----------------------------//
class SingleCell {
    private:
        std::unique_ptr<StochasticModule> stochasticModule;
        std::unique_ptr<DeterministicModule> deterministicModule;

    protected:
        virtual void _simulationPrep(
            const std::optional<std::vector<double>>& initial_state,
            double start, 
            double stop, 
            double step
        ) = 0;


        /**
         * @brief Class method for enforcing an iteration step by simulation formalism
         * 
         * @param step current step of the simulation
         * 
         * @returns None (new_state t+1 values for module step.)
        */
        virtual void runStep(
            int step
        ) = 0;    

        virtual std::vector<double> getInitialState() const = 0; //derived class implement only
        
        /**
         * @brief creates a matrix of results to be implemented within a derived class
         *      @NOTE: Method intended for derived classes only
         *
         * @param numSpecies integer number of species within the derived class model
         * @param numTimeSteps integer number of timesteps for simulation to load results to
         * 
         * @returns
         */
        std::vector<std::vector<double>> createResultsMatrix(
            int numSpecies,
            int numTimeSteps
        ); 

        /**
         * @brief calculates number of simulation steps, aka timepoints
         *
         * @param start
         * @param stop
         * @param step
         * 
         * @returns timepoints vector of float values 
         */
        std::vector<double> setTimeSteps(
            double start, 
            double stop, 
            double step
        );

        virtual void recordStepResult(
            const std::vector<double>& state_vector, 
            int timepoint
        ) = 0;

        /**
         * @brief Getter method for last recorded value in results matrix
         * 
         * @param timepoint position in results matrix being returned
         * 
         * @returns state_vector vector of species states recorded in results_matrix object
         */
        virtual std::vector<double> getLastStepResult(
            int timepoint
        ) = 0;


        virtual void updateParameters(
            const Model* alternate_model
        );

        std::vector<std::string> findOverlappingIds(
            const std::vector<std::string>& ids1,
            const std::vector<std::string>& ids2
        );


    public:
        SingleCell(
            std::string stochastic_sbml_path = "../sbml_files/Stochastic.sbml",
            std::string deterministic_sbml_path = "../sbml_files/Deterministic.sbml"
        ); //Ctor

        virtual ~SingleCell() = default; //Dtor

        std::vector<std::vector<double>> simulate(
            const std::vector<double>& det_states, //deterministic starting species values (nM)
            const std::vector<double>& stoch_states, //stochastic starting species values (nM)
            double start = 0.0, //seconds
            double stop = 60.0, //seconds
            double step = 30.0 //seconds
        );

};

#endif // SINGLECELL_H
