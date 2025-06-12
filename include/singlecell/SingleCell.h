/**
 * @file: SingleCell.h
 * 
 * @authors  Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 * 
 * @brief Class Creator For Single Cell Model.
 */
//========================header file definition============================//
#pragma once

#ifndef SINGLECELL_h
#define SINGLECELL_h

//===========================Library Import=================================//
#include <map>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <type_traits>
#include <unordered_map>

//Internal Libraries
#include "singlecell/SBMLHandler.h"
#include "singlecell/BaseModule.h"
#include "singlecell/StochasticModule.h"
#include "singlecell/DeterministicModule.h"

// Third Party Libraries
#include "sbml/SBMLReader.h"

//==========================Class Declaration===============================//
class SingleCell {
    private:
    //---------------------------methods----------------------------------//
        /**
         * @brief loads all typenamem sbml_paths into public class member handlers
         * 
         * @param sbml_paths strings of paths to SBML model files.ADD_FILTERED_PLIST
         * 
         * @returns loaded_sbmls list of SBMLHandler objects
         */
        template <typename... Paths>
        std::vector<SBMLHandler> loadSBMLModels(
            const Paths&... paths
        ) {
            std::vector<std::string> sbml_path_strings = { std::string(paths)... };
            std::vector<SBMLHandler> handler_list;

            for (const auto& path : sbml_path_strings) {
                handler_list.emplace_back(path);
            }

            return handler_list;

        }

    //---------------------------members--------------------------------//
        static std::map<std::string, std::function<std::unique_ptr<BaseModule>(const SBMLHandler&)>> moduleFactory;


    protected:
    //---------------------------methods----------------------------------//
        /**
         * @brief Factory method for loading BaseModules based on SBML details
         * 
         * @par Parameters
         * None. Loads N-instances of simulation Modules stored in class member handlers
         */
        void loadSimulationModules();

        /**
         * @brief Iterates over stored class member modules, assigns targets 
         * to modules
         */
        void assignGlobalTargets();

        /**
         * @brief Iterates over stored class member modules, identifies target entities
         * 
         */
        void findModuleOverlaps();

        /**
         * @brief modifies intial states for each module stored in class member modules
         * 
         * @param entity_map dictionary of species mapped to double values users want to change
         * @param start double start time for simulation
         * @param stop double end timepoint for simulation
         * @param step double iteration size delta-t
         */
        void setGlobalSimulationSettings(
            std::unordered_map<std::string, double>entity_map,
            double start, 
            double stop, 
            double step
        );


        /**
         * @brief class method to run modules, stepwise
         * 
         * @param timesteps vector of double steps to length of simulation
         */
        void runGlobal(
            std::vector<double> timesteps
        );

        /**
         * @brief class method to run individual simulation step
         * 
         * @param step double value of current step iteration
         */
        void stepGlobal(
            int step
        );

        void updateGlobalParameters();

        /**
         * @brief creates combined results matrix for every module in 
         * class member this->modules
         * 
         * @param numTimesteps integer number of timepoints, specifying how long 
         * results matrix should be
         * 
         * @returns results_matrix combined matrix of results for all modules, 
         * basically just adds each module implementation of BaseModule.results_matrix
         * class member to finalized results matrix.
         */
        std::vector<std::vector<double>> combineResultsMatrix(
            int numTimesteps
        );

    //------------------------------members---------------------------------//
        std::vector<std::unique_ptr<BaseModule>> modules;


    public:
    //---------------------------methods------------------------------------//
        template<typename... SBML_Paths>
        SingleCell(SBML_Paths&&... sbml_paths) {
            this->handlers = loadSBMLModels(std::forward<SBML_Paths>(sbml_paths)...);
        } //Ctor

        virtual ~SingleCell() = default; //Dtor

        /**
         * @brief public method for users to interface with the SingleCell Simulator. 
         * 
         * @param entity_map map of species to be modified, with corresponding values
         * @param start is the simulation start time
         * @param stop is the simulation stop time, in seconds
         * @param step is the delta_t step between simulation updates in seconds
         * 
         * @returns matrix of global states for both models
         */
        std::vector<std::vector<double>> simulate(
            std::unordered_map<std::string, double> entity_map,
            double start = 0.0, //seconds
            double stop = 60.0, //seconds
            double step = 30.0 //seconds
        );

        /**
         * @brief getter method for retrieving all speciesIds from all associated submodels
         * uses each model's SBMLHandler->getSpeciesIds() method.
         * 
         * @param None non-static, uses class-members
         * 
         * @returns global_ids string vector of all species identifiers from all models
         * in parent class SingleCell
         */
        std::vector<std::string> getGlobalSpeciesIds();

        //---------------------------members--------------------------------//
        std::vector<SBMLHandler> handlers;
       

};

#endif // SINGLECELL_H
