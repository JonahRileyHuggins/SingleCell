/**
 * @file: Simulation.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 19-05-2025
 * 
 * @brief Class Creator For Single Cell Model.
 */
//----------------------header file definition-----------------------------//
#pragma once

#ifndef SIMULATION_h
#define SIMULATION_h

// --------------------------Library Import--------------------------------//

#include <vector>
#include <memory>
#include <optional>

//Internal Libraries
#include "singlecell/SBMLHandler.h"

//Third Party Libraries
#include "sbml/SBMLReader.h"

//--------------------------Class Declaration-----------------------------//
class Simulation {
private:


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
     *
     * @param numSpecies integer number of species within the derived class model
     * @param numTimeSteps integer number of timesteps for simulation to load results to
     * 
     * @returns results_matrix species-by-timesteps matrix of doubles
     */
    std::vector<std::vector<double>> createResultsMatrix(
        int numSpecies,
        int numTimeSteps
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
    ) = 0;

    std::vector<std::string> findOverlappingIds(
        const std::vector<std::string>& ids1,
        const std::vector<std::string>& ids2
    );


    public:
        Simulation(
            std::unique_ptr<SBMLHandler> stochMod = nullptr,
            std::unique_ptr<SBMLHandler> detMod = nullptr
        ); //Ctor

        virtual ~Simulation() = default; //Dtor

        std::unique_ptr<SBMLHandler> StochasticModel;
        std::unique_ptr<SBMLHandler> DeterministicModel;

        /**
         * @brief calculates number of simulation steps, aka timepoints
         *
         * @param start
         * @param stop
         * @param step
         * 
         * @returns timepoints vector of float values 
         */
        static std::vector<double> setTimeSteps(
            double start, 
            double stop, 
            double step
        );

        /**
         * @brief Concatenates matrix 2 to the bottom rows of matrix 1
         * 
         * @param matrix1 nested vector of doubles matrix to be appended to
         * @param matrix2 nested vector of doubles matrix to be appended
         * 
         * @returns combined_matrix matrix of (matrix1 row count + matrix2 row count)
         */
        static std::vector<std::vector<double>> concatenateMatrixRows(
            
            std::vector<std::vector<double>> matrix1, 
            std::vector<std::vector<double>> matrix2
        );
        
};

#endif // SIMULATION_H
    