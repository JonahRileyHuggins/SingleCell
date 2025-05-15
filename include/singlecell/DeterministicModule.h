/**
 * @file: DeterministicModule.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 14-05-2025
 * 
 * @brief Class Creator For Deterministic Module Using AMICI
 */

//--------------helper function definition------------------------------------//

#ifndef DETERMINISTICMODULE_h
#define DETERMINISTICMODULE_h

// --------------------------Library Import-----------------------------------//
#include <vector>
#include <memory>

//Internal Libraries
#include "singlecell/SingleCell.h"

// Third Party Libraries
#include "amici/amici.h"

//--------------------------Class Declaration-----------------------------//
class DeterministicModule : public SingleCell {
    public:
        DeterministicModule(
            const std::string* sbml_path
        ); //Ctor

        ~DeterministicModule() override = default; //Dtor

        std::vector<double> runStep(
            const std::vector<double>& state_vector
        ) override;

        void exchangeData() override;

        std::vector<std::vector<double>> createResultsMatrix(
            int numSpecies,
            double start, 
            double stop,
            double step
        ) override;

    private:
        std::unique_ptr<SBMLHandler> sbmlHandler;
        std::vector<std::vector<double>> stoichmat;
        std::vector<std::string> formulas_vector;
        std::unique_ptr<amici::Model> model;
        std::unique_ptr<amici::Solver> solver;

        std::vector<double> setAllSpeciesValues(
            std::vector<double> current_states,
            std::vector<double> update_states
        );

        std::vector<double> getLastValues(
            const amici::ReturnData &rdata
        );

};

#endif