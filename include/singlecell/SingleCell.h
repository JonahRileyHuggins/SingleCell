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
#include <optional>

//Internal Libraries
#include "sbml/SBMLReader.h"
#include "singlecell/Simulation.h"
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"

//--------------------------Class Declaration-----------------------------//
class SingleCell {
    private:

    protected:

    public:
        SingleCell(
            std::string stochastic_sbml_path = "../sbml_files/Stochastic.sbml",
            std::string deterministic_sbml_path = "../sbml_files/Deterministic.sbml"
        ); //Ctor

        virtual ~SingleCell() = default; //Dtor

        std::vector<std::vector<double>> simulate(
            const std::vector<double>& det_states, //deterministic starting species values (nM)
            const std::vector<double>& stoch_states, //stochastic starting species values (nM)
            double start = 0.0, //seconds
            double stop = 60.0, //seconds
            double step = 30.0 //seconds
        );

        std::unique_ptr<SBMLHandler> StochasticModel;
        std::unique_ptr<SBMLHandler> DeterministicModel;
       

};

#endif // SINGLECELL_H
