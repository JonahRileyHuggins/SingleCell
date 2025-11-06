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
#include <Eigen/Dense>

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
         * @brief Calculates a single timestep for the Deterministic module
         * 
         * @param step current step of the simulation
         * 
         * @returns None (new state vector of t+1 values for Deterministic step)
        */
        void step(
            int step
        ) override;

        /**
         * @brief Calculates every timestep for the Deterministic module before returning call
         * 
         * @param timepoints vector of timepoints for the simulation
         */
        void run(
            Eigen::VectorXd timepoints
        ) override;

        /**
         * @brief exchanges parameter-to-species values with target-modules
         * 
         */
        void updateParameters();
        
    //-------------------------------Members--------------------------------//
        std::string algorithm_id;


    private:
    // ---------------------------Methods-----------------------------------//
        Eigen::VectorXd getNewStepResult(
            const amici::ReturnData &rdata
        );

        /**
         * @brief internal method to update AMICI model parameters
         */
        void updateAMICIModel();
        
    //-------------------------------Members--------------------------------//
        std::unique_ptr<amici::Model> model;
        std::unique_ptr<amici::Solver> solver;


    protected:
    // ---------------------------Methods-----------------------------------//

    //-------------------------------Members--------------------------------//


};

#endif