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

        void _simulationPrep(
            double start,
            double stop, 
            double step
        );

        std::vector<double> runStep(
            const std::vector<double>& state_vector
        ) override;

        void setModelState(
            const std::vector<double>& state
        );

        void exchangeData() override;


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
        void recordTimeStep(
            const std::vector<double>& state_vector,
            int timepoint
        );

};

#endif // STOCHASTICMODULE_H
