/**
 * @file: StochasticModule.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Class Creator For Single Cell Model, Stochastic Module Instance.
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
#include "SingleCell.h"
#include "SBMLHandler.h"

//--------------------------Class Declaration-----------------------------//
class StochasticModule : public SingleCell {
    public:
        StochasticModule( //Constructor. ctor
            const std::string& sbml_path
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
            const std::optional<std::vector<double>>& initial_state,
            double start,
            double stop, 
            double step
        ) override;

        
        std::vector<double> runStep(
            const std::vector<double>& state_vector
        ) override;

        void setModelState(
            const std::vector<double>& state
        );

        void exchangeData() override;

        virtual std::vector<double> getInitialState() const override;

    private:
        std::unique_ptr<SBMLHandler> sbmlHandler;
        std::vector<std::vector<double>> stoichmat;
        std::vector<std::string> formulas_vector;
        std::vector<std::vector<double>> results_matrix;

        
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
            std::vector<double> initial_reaction_vector
        );

        protected:
        void recordStepResult(
            const std::vector<double>& state_vector,
            int timepoint
        ) override;

};

#endif // STOCHASTICMODULE_H
