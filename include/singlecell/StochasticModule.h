/*
filename: StochasticModule.h
created by: Jonah R. Huggins
created on: 25-05-15

description: 
Class Creator For Single Cell Model, Stochastic Module Instance.
*/
//--------------header file definition------------------------------------//
#pragma once;

#ifndef STOCHASTICMODULE_h
#define STOCHASTICMODULE_h

// --------------------------Library Import-----------------------------------//
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
            const std::string& sbml_path = "../sbml_files/Stochastic.sbml", 
            double timeStep = 30.0
        );

        ~StochasticModule() override = default; //Destructor, dtor

        std::vector<double> runStochasticStep(
            const std::vector<double>& stochastic_states
        );

        void setModelState(
            const std::vector<double>& state
        );

    private:
        std::unique_ptr<StochasticModule> StochMod_;
        std::unique_ptr<SBMLHandler> sbmlHandler;
        std::vector<std::vector<double>> stoichmat;
        std::vector<std::string> formulas_vector;
        double step;

        
        std::vector<double> computeReactions(
            const std::vector<double>& state
        );

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
        void exchangeData() override;
};


#endif // STOCHASTICMODULE_H
