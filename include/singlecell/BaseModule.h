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

    //-------------------------------Members--------------------------------//
        std::string algorithm_id = "";
        std::string target_id = "";
        double delta_t;


    public:
    //---------------------------methods------------------------------------//
        BaseModule(
            SBMLHandler Module
        ); //Ctor

        virtual ~BaseModule() = default; //Dtor

        /**
         * @brief retrieves private member algorithm_id for determining which simulation
         * method to use
         *  
         * @returns algorithm_id string identifier for algorithm default
         */
        virtual std::string getModuleId() = 0;   

        /**
         * @brief finds targets for module to send information to.ADD_FILTERED_PLIST
         * 
         */
        virtual void loadTargetModule(
            const std::vector<std::unique_ptr<BaseModule>>& module_list
        ) = 0; 

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
         * @brief Enforces an iterative, step-wise simulation method in derived classes
         * 
         * @param step current step of the simulation
         * 
         * @returns None (new_state t+1 values for module step.)
        */
        virtual void step(
            int step
        ) = 0; 

        /**
         * @brief Enforces a method for simulating without stop in derived classes
         * 
         * @param timepoints vector of timepoints for the simulation
         */
        virtual void run(
            std::vector<double> timepoints
        ) = 0;        

        void findOverlappingIds(
            const Model* alternate_model
        );
    
        virtual void setSimulationSettings(
            double start, 
            double stop, 
            double step
        ) = 0;

        /**
         * @brief Exchange method for swapping parameters with species values in modules
         *  @NOTE: required method in Stochastic- & Deterministic- Modules
         * 
         * @returns None updates internal models.
         */
        virtual void updateParameters() = 0;

    //-------------------------------Members--------------------------------//
        SBMLHandler handler;
        Model* sbml;

        std::vector<std::vector<double>> stoichmat;

        std::vector<std::string> formulas_vector;

        std::vector<std::vector<double>> results_matrix;

        std::vector<std::string> overlapping_params;

        std::vector<BaseModule*> targets;

        std::vector<double> timesteps;


};

#endif // BASEMODULE_H
    