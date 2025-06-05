/**
 * @file main.cpp
 * 
 * @brief entrypoint file for simulation in C++
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 20-05-2025
 */

 // --------------------------Library Import--------------------------------//
#include <unordered_map>
 #include <any>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>


// Internal Libraries
#include "singlecell/utils.h"
#include "singlecell/ArgParsing.h"
#include "singlecell/SingleCell.h"

//--------------------------Function Definitions----------------------------//

/**
 * @brief executes simulation instructions
 * 
 * @param
 * 
 * @returns None
 */
int main(
    int argc, 
    char* argv[]
) {

    std::unique_ptr<ArgParsing> argparser = std::make_unique<ArgParsing>(
        argc, argv
    );

    std::unordered_map<std::string, std::any> cli_map = argparser->cli_map;

    double start = std::any_cast<double>(argparser->cli_map["--start"]);
    double stop = std::any_cast<double>(argparser->cli_map["--stop"]);
    double step = std::any_cast<double>(argparser->cli_map["--step"]);

    std::string stochastic_sbml = std::any_cast<std::string>(argparser->cli_map["--stochastic_model"]);
    std::string deterministic_sbml = std::any_cast<std::string>(argparser->cli_map["--deterministic_model"]);

    //Load instance of SingleCell
    std::unique_ptr<SingleCell> single_cell = std::make_unique<SingleCell>(
        stochastic_sbml,
        deterministic_sbml
    );
    std::cout << "Entity Map:\n";
    for (const auto& [key, value] : argparser->cli_map) {
        std::cout << "  " << key << " => ";
        try {
            std::cout << std::any_cast<std::string>(value);
        } catch (std::bad_any_cast) {
            std::cout << std::any_cast<double>(value);
        } 
        std::cout << '\n';
    }
    std::vector<std::vector<double>> results_matrix = single_cell->simulate(
        argparser->entity_map,
        start, 
        stop, 
        step
    );

    std::vector<std::string> timesteps(results_matrix.size());

    for (int i = 0; i < results_matrix.size(); i++) {

        double time_i = i * step;

        timesteps[i] = std::to_string(time_i);

    }

    std::vector<std::string> global_species_ids = single_cell->getGlobalSpeciesIds();

    matrix_utils::save_matrix(
        results_matrix,
        std::any_cast<std::string>(argparser->cli_map["--output"]),
        timesteps,
        global_species_ids
    );

    return 0;
}