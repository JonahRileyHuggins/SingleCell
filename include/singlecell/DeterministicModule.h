/**
 * @file: DeterministicModule.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 14-05-2025
 * 
 * @brief Class Creator For Deterministic Module Using AMICI
 */

//--------------helper function definition------------------------------------//
#pragma once

#ifndef DETERMINISTICMODULE_h
#define DETERMINISTICMODULE_h

// --------------------------Library Import-----------------------------------//
#include <vector>
#include <memory>
#include <optional>

//Internal Libraries
#include "Simulation.h"
#include "SBMLHandler.h"

// Third Party Libraries
#include "amici/amici.h"

//--------------------------Class Declaration-----------------------------//
class DeterministicModule : public Simulation {
    public:
        DeterministicModule(
            SBMLHandler DeterministicModel
        ); //Ctor

        ~DeterministicModule() override = default; //Dtor

        std::vector<std::vector<double>> results_matrix;

        void _simulationPrep(
            const std::optional<std::vector<double>>& initial_state,
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
            const Model* alternate_model
        );

        virtual std::vector<double> getInitialState() const override;

        Model* sbml;

    private:
        std::vector<std::vector<double>> stoichmat;
        std::vector<std::string> formulas_vector;
        std::unique_ptr<amici::Model> model;
        std::unique_ptr<amici::Solver> solver;

        std::vector<double> setAllSpeciesValues(
            std::vector<double> current_states,
            std::vector<double> update_states
        );

        std::vector<double> getNewStepResult(
            const amici::ReturnData &rdata
        );



    protected:
        void recordStepResult(
            const std::vector<double>& state_vector,
            int timepoint
        ) override;

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

};

#endif