/**
 * @file: StochasticModule.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Class Creator For derived class StochasticModule (from BaseModule).
 */
//========================header file definition============================//
#pragma once

#ifndef STOCHASTICMODULE_h
#define STOCHASTICMODULE_h

//===========================Library Import=================================//
//Std Libaries
#include <vector>
#include <memory>
#include <random>
#include <optional>
#include <unordered_map>

// Internal libraries
#include "BaseModule.h"
#include "SBMLHandler.h"

// External Libraries
#include "parser.h"
#include <Eigen/Dense>

//==========================Class Declaration===============================//
class StochasticModule : public BaseModule{
    public:
    // -------------------------Methods-----------------------------------//
        StochasticModule( //Constructor. ctor
            SBMLHandler StochasticModel
        );

        ~StochasticModule() override = default; //Destructor, dtor

        /**
         * @brief retrieves private member algorithm_id for determining which simulation
         * method to use
         *  
         * @returns algorithm_id string identifier for algorithm default
         */
        std::string getModuleId() override;  

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
        void setSimulationSettings(
            double start,
            double stop, 
            double step
        ) override;

        
        /**
         * @brief Calculates a single timestep for the stochastic module, returning to call
         * 
         * @param step current step of the simulation
         * 
         * @returns new_state t+1 values for stochastic step.
        */
        void step(
            int step
        ) override;

        /**
         * @brief Calculates every timestep for the stochastic module before returning call
         * 
         * @param timepoints vector of timepoints for the simulation
         */
        void run(
            Eigen::VectorXd timepoints
        ) override;

        /**
         * @brief Override class for BaseModule, exchanges data with target
         * modules at each timestep
         */
        void updateParameters();
        
    //---------------------------Members----------------------------------//
        std::string algorithm_id;    


    private:
    // -------------------------Methods-----------------------------------//

        /**
         * @brief Computes reactions for the most recent time step
         * 
         * @returns new state vector for current step
         */
        Eigen::VectorXd computeReactions();


        /**
        * @brief class instance to calculate a reaction formula using the muParser object
        * 
        * @param formula_str reaction formula to be calculated
        * 
        * @return v_i reaction i's left hand result
        */
        double computeReaction(
            const std::string &formula_str
        );

        /**
         * @brief Finds all species in the formula string 
         * 
         * @param formula_str string form of the reaction formula
         * 
         * @returns Map of component IDs to their numerical value
         */
        std::unordered_map<std::string,double> getFormulaValues(
            const std::string& formula_str
        );

        /**
         * @brief extended cctypes isalnum() method that also allows underscore characters
         * 
         * @param c character being evaluated
         * 
         * @returns 0 || 1
         */
        bool is_alnumus(char c);

        /**
         * @brief swaps a string containing alphanumeric characters and underscores with
         * another string
         * 
         * @param input the string to be modified
         * @param swap the substring within input to be swapped out
         * @param with the substring to replace the swap-string
         * 
         * @returns input the now-modified input string
         */
        std::string safe_replace_alnumus(
            std::string &input, 
            const std::string &swap,
            double with_val
            );
        
        /**
         * @brief converts double precision values to a 15th decimal string
         * @note standard
         * @param val double value to be converted
         * 
         * @returns proper double to 15th decimal place
         */
        std::string to_str(double val);

        /** 
         * @brief Update stoichiometric values by setting as the mean for a poission distribution
         * 
         * @param mu the rate vector calculated from each reaction, per second time unit
         * 
         * @returns m_i vector of Poisson-dist informed scalar values for righthand side v of x_dot = S*v
        */
        Eigen::VectorXd samplePoisson(
            Eigen::VectorXd mu
        );

        /**
         * @brief constrains Tau leap against negative values that result from low copy numbers
         * 
         * @param xhat_tn current poisson-sample vector
         * 
         * @returns  m_actual minimum choice between negative reactants per reaction
        */
        Eigen::VectorXd constrainTau(
            Eigen::VectorXd m_i,
            Eigen::VectorXd xhat_tn
        ); 

        /**
         * @brief calculates the updated state by adding to the prior state
         *  the new rates (Schilling and Palsson, 1998)
         * 
         * @param state_t the vector of states (in molecules) of the prior timestep
         * @param real_vec vector of propensity realizations, sampled from a poisson dist. 
         * and constrained to perserve moiety
         * 
         * @returns new_state vector of doubles equal to X_t = X_{t-1} + delta
         */
        Eigen::VectorXd computeNewState(
        Eigen::VectorXd state_t,
        Eigen::VectorXd real_vec
        );

        //---------------------------Members----------------------------------//
        std::unordered_map<std::string, std::vector<std::string>> tokenized_formula_map;
        Eigen::VectorXd molecules2nM_conversion_factors;
        Eigen::VectorXd nM2mpv_conversion_factors;
        Eigen::VectorXd species_volumes;
        std::mt19937 generator;

    protected:
        // -------------------------Methods-----------------------------------//

        //---------------------------Members----------------------------------//


};

#endif // STOCHASTICMODULE_H
