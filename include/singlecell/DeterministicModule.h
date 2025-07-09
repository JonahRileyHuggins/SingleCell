/**
 * @file: DeterministicModule.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 14-05-2025
 * 
 * @brief Class Creator For Deterministic Module Using AMICI
 */

//========================header file definition============================//
#pragma once

#ifndef DETERMINISTICMODULE_h
#define DETERMINISTICMODULE_h

//===========================Library Import=================================//
//Std Libraries
#include <vector>
#include <memory>
#include <optional>

//Internal Libraries
#include "BaseModule.h"
#include "SBMLHandler.h"

// Third Party Libraries
#include "amici/amici.h"

//==========================Class Declaration===============================//
class DeterministicModule : public BaseModule {
    public:
    //----------------------------Methods-----------------------------------//
        DeterministicModule(
            SBMLHandler DeterministicModel
        ); //Ctor

        ~DeterministicModule() override = default; //Dtor

        /**
         * @brief retrieves private member algorithm_id for determining which simulation
         * method to use
         *  
         * @returns algorithm_id string identifier for algorithm default
         */
        std::string getModuleId() override;  

        void setSimulationSettings(
            double start,
            double stop, 
            double step
        ) override;

        /**
         * @brief Calculates a single timestep for the deterministic module
         * 
         * @param step current step of the simulation
         * 
         * @returns None (new state vector of t+1 values for deterministic step)
        */
        void step(
            int step
        ) override;

        /**
         * @brief Calculates every timestep for the deterministic module before returning call
         * 
         * @param timepoints vector of timepoints for the simulation
         */
        void run(
            std::vector<double> timepoints
        ) override;

        /**
         * @brief exchanges parameter-to-species values with target-modules
         * 
         */
        void updateParameters();
        
    //-------------------------------Members--------------------------------//
        std::string algorithm_id = "Deterministic";


    private:
    // ---------------------------Methods-----------------------------------//
        std::vector<double> setAllSpeciesValues(
            std::vector<double> current_states,
            std::vector<double> update_states
        );

        std::vector<double> getNewStepResult(
            const amici::ReturnData &rdata
        );
        
    //-------------------------------Members--------------------------------//
        std::unique_ptr<amici::Model> model;
        std::unique_ptr<amici::Solver> solver;


    protected:
    // ---------------------------Methods-----------------------------------//
        /**
         * @brief Getter method for last recorded value in results matrix
         * 
         * @param timepoint position in results matrix being returned
         * 
         * @returns state_vector vector of species states recorded in results_matrix object
         */
        std::vector<double> getLastStepResult(
            int timestep
        ) override;

        void loadTargetModule(
            const std::vector<std::unique_ptr<BaseModule>>& module_list
        ) override;

    //-------------------------------Members--------------------------------//


};

#endif