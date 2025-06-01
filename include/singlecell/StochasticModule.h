/**
 * @file: StochasticModule.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Class Creator For derived class StochasticModule (from Simulator).
 */
//========================header file definition============================//
#pragma once

#ifndef STOCHASTICMODULE_h
#define STOCHASTICMODULE_h

//===========================Library Import=================================//
//Std Libaries
#include <vector>
#include <memory>
#include <optional>
#include <unordered_map>

// Internal libraries
#include "Simulation.h"
#include "SBMLHandler.h"

//==========================Class Declaration===============================//
class StochasticModule : public Simulation {
    public:
    // -------------------------Methods-----------------------------------//
        StochasticModule( //Constructor. ctor
            SBMLHandler StochasticModel
        );

        ~StochasticModule() override = default; //Destructor, dtor

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
            std::unordered_map<std::string, double>entity_map,
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

        /**
         * @brief public method for updating the simulation states at every timestep. 
         * 
         * @param state vector of timestep values to be calculated. 
         * 
         * @returns None
         */
        void setModelState(
            const std::vector<double>& state
        );

        void updateParameters(
            const Model* alternate_model
        );
    //---------------------------Members----------------------------------//


    private:
    // -------------------------Methods-----------------------------------//
        std::vector<double> computeReactions(
            const std::vector<double>& state
        );


        /**
        * @brief class instance to calculate a reaction formula using the muParser object
        * 
        * @param formula_str reaction formula to be calculated
        * 
        * @return v_i reaction i's left hand result
        */
        double computeReaction(
            std::string formula_str
        );

        /**
         * @brief Finds all species in the formula string 
         *      NOTE: species must be initialConcentration only. 
         *      TODO: make compatible with Species initialAmount
         * 
         * @param formula_str string form of the reaction formula
         * 
         * @returns Map of component IDs to their numerical value
         */
        std::unordered_map<std::string,double> mapComponentsToValues(
            const std::string& formula_str
        );
        
        /**
         * @brief Creates a list of strings based on formula contents
         * 
         * @param formula_str string type reaction formula
         * 
         * @returns tokens a list of components from the formula, without operators
         */
        std::vector<std::string> tokenizeFormula(
            const std::string& formula_str
        );

        /** 
         * @brief Update stoichiometric values by setting as the mean for a poission distribution
         * 
         * @param mu the rate vector calculated from each reaction, per second time unit
         * 
         * @returns m_i vector of Poisson-dist informed scalar values for righthand side v of x_dot = S*v
        */
        std::vector<double> samplePoisson(
            std::vector<double> mu
        );

        /**
         * @brief constrains Tau leap against negative values that result from low copy numbers
         * 
         * @param xhat_tn current poisson-sample vector
         * 
         * @returns  m_actual minimum choice between negative reactants per reaction
        */
        std::vector<double> constrainTau(
            std::vector<double> m_i,
            std::vector<double> xhat_tn
        ); 

        
        //---------------------------Members----------------------------------//
        std::vector<std::vector<double>> stoichmat;
        std::vector<std::string> formulas_vector;
        double delta_t; 


        protected:
        // -------------------------Methods-----------------------------------//
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

        //---------------------------Members----------------------------------//
        

};

#endif // STOCHASTICMODULE_H
