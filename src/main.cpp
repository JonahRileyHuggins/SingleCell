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
    std::unique_ptr<SingleCell> singleCell = std::make_unique<SingleCell>(
        stochastic_sbml,
        deterministic_sbml
    );

    std::vector<std::vector<double>> results_matrix = singleCell->simulate(
        argparser->entity_map,
        start, 
        stop, 
        step
    );


    matrix_utils::save_matrix(results_matrix);

    return 0;
}