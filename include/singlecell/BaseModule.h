/**
 * @file: BASEMODULE.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 19-05-2025
 * 
 * @brief Base-Class Creator For individual simulation modules. New Simulation 
 * formalisms should become derived classes
 */
//========================header file definition============================//
#pragma once

#ifndef BASEMODULE_h
#define BASEMODULE_h

//===========================Library Import=================================//
//Std Libraries
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>

//Internal Libraries
#include "singlecell/SBMLHandler.h"

//Third Party Libraries
#include "sbml/SBMLReader.h"

//==========================Class Declaration===============================//
class BaseModule {
    private:
    //---------------------------------methods------------------------------//


    //-------------------------------Members--------------------------------//


    protected:
    //---------------------------------methods------------------------------//
        /**
         * @brief retrieves private member algorithm_id for determining which simulation
         * method to use
         *  
         * @returns algorithm_id string identifier for algorithm default
         */
        std::string getModuleId();
    
        virtual void _simulationPrep(
            std::unordered_map<std::string, double>entity_map,
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

        /**
         * @brief Records timestep to internal results matrix
         * 
         * @param state_vector array of model states at timepoint
         * @param timepoint timepoint in simulation states were recorded for
         * 
         * @returns None assigns new states to member results_matrix
         */
        void recordStepResult(
            const std::vector<double>& state_vector, 
            int timepoint
        );

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

        /**
         * @brief Exchange method for swapping parameters with species values in modules
         *  @NOTE: required method in Stochastic- & Deterministic- Modules
         * 
         * @param alternate_model the model for which species values correspond to 
         * parameter values in this-> model
         * 
         * @returns None updates internal models.
         */
        virtual void updateParameters(
            SBMLHandler alternate_model
        ) = 0;

    //-------------------------------Members--------------------------------//
        std::string algorithm_id = "Deterministic";


    public:
    //---------------------------methods------------------------------------//
        BaseModule(
            SBMLHandler Module
        ); //Ctor

        virtual ~BaseModule() = default; //Dtor

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

        void modifyModelEntity(
            std::string entity_id, 
            double new_value
        );
        

        void findOverlappingIds(
            const Model* alternate_model
        );

    //-------------------------------Members--------------------------------//
        SBMLHandler handler;
        Model* sbml;

        std::vector<std::vector<double>> stoichmat;
        std::vector<std::string> formulas_vector;

        std::vector<std::vector<double>> results_matrix;

        std::vector<std::string> overlapping_params;


};

#endif // BASEMODULE_H
    