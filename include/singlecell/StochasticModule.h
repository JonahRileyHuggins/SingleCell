/**
 * @file: StochasticModule.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Class Creator For derived class StochasticModule (from Simulator).
 */
//----------------------header file definition-----------------------------//
#pragma once

#ifndef STOCHASTICMODULE_h
#define STOCHASTICMODULE_h

// --------------------------Library Import--------------------------------//
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>

// Internal libraries
#include "Simulation.h"
#include "SBMLHandler.h"

//--------------------------Class Declaration-----------------------------//
class StochasticModule : public Simulation {
    public:
        StochasticModule( //Constructor. ctor
            SBMLHandler StochasticModel
        );

        ~StochasticModule() override = default; //Destructor, dtor

        std::vector<std::vector<double>> results_matrix;

        /**
         * @brief loads pre-simulation materials: results matrix, [Fill in here Jonah]
         *      @TODO: Are we adding results matrix to the object? If so, create a fill in holder.computeReaction
         * 
         * @param initial_state
         * @param start
         * @param stop
         * @param step
         * 
         * @returns None
         */
        void _simulationPrep(
            const std::optional<std::vector<double>>& initial_state,
            double start,
            double stop, 
            double step
        ) override;

        
        /**
         * @brief Calculates a single timestep for the stochastic module
         * 
         * @param step current step of the simulation
         * 
         * @returns new_state t+1 values for stochastic step.
        */
        void runStep(
            int step
        ) override;

        void setModelState(
            const std::vector<double>& state
        );

        void updateParameters(
            const Model* alternate_model
        );

        virtual std::vector<double> getInitialState() const override;

        Model* sbml;

    private:
        std::vector<std::vector<double>> stoichmat;
        std::vector<std::string> formulas_vector;
        
        std::vector<double> computeReactions(
            const std::vector<double>& state
        );

        double computeReaction(
            std::string formula_str
        );

        std::unordered_map<std::string,double> mapComponentsToValues(
            const std::string& formula_str
        );
        
        std::vector<std::string> tokenizeFormula(
            const std::string& formula_str
        );

        std::vector<double> samplePoisson(
            std::vector<double> initial_reaction_vector,
            int step
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

#endif // STOCHASTICMODULE_H
