/**
 * @file: SingleCell.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Class Creator For Single Cell Model.
 */
//----------------------header file definition-----------------------------//
#pragma once

#ifndef SINGLECELL_h
#define SINGLECELL_h

// --------------------------Library Import--------------------------------//

#include <vector>
#include <memory>

//Internal Libraries
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"

//--------------------------Class Declaration-----------------------------//
class SingleCell {
    private:
        std::unique_ptr<StochasticModule> stochasticModule;
        std::unique_ptr<DeterministicModule> deterministicModule;

    protected:
        virtual void _simulationPrep(
            double start, 
            double stop, 
            double step
        ) = 0;

        virtual std::vector<double> runStep(
            const std::vector<double>& state_vector
        ) = 0;    

        std::vector<std::vector<double>> createResultsMatrix(
            int numSpecies,
            int numTimeSteps
        ); 

        virtual void exchangeData();

        std::vector<double> setTimeSteps(
            double start, 
            double stop, 
            double step
        );

        virtual void recordTimeStep(
            const std::vector<double>& state_vector, 
            int timepoint
        ) = 0;

    public:
        SingleCell(
            std::string stochastic_sbml_path = "../sbml_files/Stochastic.sbml",
            std::string deterministic_sbml_path = "../sbml_files/Deterministic.sbml"
        ); //Ctor

        virtual ~SingleCell(); //Dtor

        std::vector<std::vector<double>> simulate(
            const std::vector<double>& det_states, //deterministic starting species values (nM)
            const std::vector<double>& stoch_states, //stochastic starting species values (nM)
            double start = 0.0, //seconds
            double stop = 60.0, //seconds
            double step = 30.0 //seconds
        );

};

#endif // SINGLECELL_H
