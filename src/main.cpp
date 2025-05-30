/**
 * @file main.cpp
 * 
 * @brief entrypoint file for simulation in C++
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 20-05-2025
 */

 // --------------------------Library Import--------------------------------//
#include <any>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>

// Internal Libraries
#include "singlecell/utils.h"
#include "singlecell/SingleCell.h"

//--------------------------Function Definitions----------------------------//

double parseTimeArgs(
    int argc, 
    char* argv[], 
    int index, 
    double def, 
    const char* arg_name
) {
    if (argc > index) { // if arguement is greater than provided index, 

        char* end = nullptr; // make end point

        double v = std::strtod(argv[index], &end); // convert argv @ index to a floating point number, with end pointer being null

        if (end == argv[index] || *end != '\0') { // If argv @ index is null OR the pointer to end does NOT equal null terminator:

            std::cerr << "Bad" << arg_name << " value: " << argv[index] << '\n';
            std::exit(EXIT_FAILURE);
        }
        return v;
    }

    return def;
}

std::string parsePathArgs(
    int argc, 
    char* argv[],
    int index, 
    std::string def,
    const char* arg_name
) {
    if (argc > index) {

        char* end = nullptr;
        
        std::string path = argv[index];
    
        return path;
        if (end == argv[index] || *end != '\0') { // If argv @ index is null OR the pointer to end does NOT equal null terminator:

            std::cerr << "Bad" << arg_name << " value: " << argv[index] << '\n';
            std::exit(EXIT_FAILURE);
        }
    }

    return def;
}

/**
 * @brief executes simulation instructions
 * 
 * @param
 * 
 * @returns None
 */
int main(
    int argc, // OS-defined parameter for CLI-argument count
    char* argv[] //OS-defined parameter for CLI-arguements as a vector of char*, representing individual CLI args.
) {

    double start = 0.0;
    double stop = 60.0; 
    double step = 30.0;
    std::string stochastic_sbml = "../tests/Stochastic.sbml";
    std::string deterministic_sbml = "../tests/Deterministic.sbml";

    start = parseTimeArgs(argc, argv, 1, start, "start");
    stop = parseTimeArgs(argc, argv, 2, stop, "stop");
    step = parseTimeArgs(argc, argv, 3, step, "step");
    stochastic_sbml = parsePathArgs(argc, argv, 4, stochastic_sbml, "stochastic sbml path");
    deterministic_sbml = parsePathArgs(argc, argv, 4, deterministic_sbml, "deterministic sbml path");

    //Load instance of SingleCell
    std::unique_ptr<SingleCell> singleCell = std::make_unique<SingleCell>(
        stochastic_sbml,
        deterministic_sbml
    );

    std::vector<std::vector<double>> results_matrix = singleCell->simulate({}, {}, 
                                                                    start, stop, step);



    matrix_utils::save_matrix(results_matrix);

    return 0;
}