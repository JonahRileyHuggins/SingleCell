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

// Third Party Libraries
#include <Eigen/Dense>

// Internal Libraries
#include "utils.h"
#include "ArgParsing.h"
#include "SingleCell.h"

//--------------------------Function Definitions----------------------------//

void save_matrix(
    const Eigen::MatrixXd& results_matrix,
    const std::string& output,
    const std::vector<std::string>& row_labels = {},
    const std::vector<std::string>& col_labels = {}
) {
    std::ofstream outFile(output);

    int numRows = results_matrix.rows();
    int numCols = results_matrix.cols();

    // Write column labels
    if (!col_labels.empty()) {
        outFile << "index";
        for (const auto& label : col_labels) {
            outFile << "\t" << label;
        }
        outFile << "\n";
    }

    // Write each row
    for (int i = 0; i < numRows; ++i) {
        if (!row_labels.empty()) {
            outFile << row_labels[i];
        } else {
            outFile << i;  // optional: write row index if no label
        }

        for (int j = 0; j < numCols; ++j) {
            outFile << "\t" << results_matrix(i, j);
        }

        outFile << "\n";
    }

    outFile.close();
}


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

    std::cout << "Simulation Details:\n";
    for (const auto& [key, value] : argparser->cli_map) {
        std::cout << "  " << key << " => ";
        try {
            std::cout << std::any_cast<std::string>(value);
            
        } catch (std::bad_any_cast) {
            std::cout << std::any_cast<double>(value);
        } 
        std::cout << '\n';
    }

        // modify sbml model prior to AMICI-model assignment
    std::vector<double> init_states;

    if (argparser->entity_map.empty()) {
        printf("Using default model states for simulation.\n");
    } else { 
        for (const auto& [key, value] : argparser->entity_map) {
            single_cell->modify(
                key, 
                value
            );
        }
    }
    Eigen::MatrixXd results_matrix = single_cell->simulate(
        start, 
        stop, 
        step
    );

    std::vector<std::string> timesteps(results_matrix.rows());

    for (int i = 0; i < results_matrix.rows(); i++) {

        double time_i = i * step;

        timesteps[i] = std::to_string(time_i);

    }

    std::vector<std::string> global_species_ids = single_cell->getGlobalSpeciesIds();

    save_matrix(
        results_matrix,
        std::any_cast<std::string>(argparser->cli_map["--output"]),
        timesteps,
        global_species_ids
    );

    return 0;
}