/**
 * @file: One4AllModule.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 14-05-2025
 * 
 * @brief Class Creator For One4All Module Using AMICI
 */

//========================header file definition============================//
#pragma once

#ifndef One4AllMODULE_h
#define One4AllMODULE_h

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
class One4AllModule : public BaseModule {
    public:
    //----------------------------Methods-----------------------------------//
        One4AllModule(
            SBMLHandler One4AllModel
        ); //Ctor

        ~One4AllModule() override = default; //Dtor

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
         * @brief Calculates a single timestep for the One4All module
         * 
         * @param step current step of the simulation
         * 
         * @returns None (new state vector of t+1 values for One4All step)
        */
        void step(
            int step
        ) override;

        /**
         * @brief Calculates every timestep for the One4All module before returning call
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
         * @brief internal method for assigning updated parameter values to the 
         * AMICI models
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