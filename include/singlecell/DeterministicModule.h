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

        void _simulationPrep(
            std::unordered_map<std::string, double>entity_map,
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
        void runStep(
            int step
        ) override;

        void updateParameters(
            SBMLHandler alternate_model
        );
    //-------------------------------Members--------------------------------//


    private:
    // ---------------------------Methods-----------------------------------//
        std::unique_ptr<amici::Model> model;
        std::unique_ptr<amici::Solver> solver;

        std::vector<double> setAllSpeciesValues(
            std::vector<double> current_states,
            std::vector<double> update_states
        );

        std::vector<double> getNewStepResult(
            const amici::ReturnData &rdata
        );
        
    //-------------------------------Members--------------------------------//


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

    //-------------------------------Members--------------------------------//


};

#endif