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
#include <type_traits>
#include <functional>

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
            std::vector<SBMLHandler> hander_list;

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

    //------------------------------members---------------------------------//
        std::vector<std::unique_ptr<BaseModule>> modules;


    public:
    //---------------------------methods------------------------------------//
        template <typename... SBML_Paths>
        SingleCell(
            SBML_Paths... sbml_paths
        ) {

            static_assert(std::conjunction_v<std::is_convertible<sbml_paths, std::string>,
                        "All sbml paths to SingleCell must be convertible to std::string");

            this->handlers = loadSBMLModels(const SBML_Paths&... sbml_paths);

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
