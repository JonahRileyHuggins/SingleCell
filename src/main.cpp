/**
 * @file main.cpp
 * 
 * @brief entrypoint file for simulation in C++
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 20-05-2025
 */

 // --------------------------Library Import--------------------------------//
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>

// Internal Libraries
#include "singlecell/SingleCell.h"

//--------------------------Function Definitions----------------------------//
/**
 * @brief Saves results matrix from class SingleCell method Simulate
 * 
 * @param results_matrix matrix of species states by time in simulation function
 * 
 * @returns none
 */
void save_results(
    std::vector<std::vector<double>> results_matrix
) {

    // Results file:
    std::string outfile = "results.txt";

    std::ofstream outFile(outfile);

    int numRows = results_matrix.size();

    int numCols = results_matrix[0].size();

    for (size_t i = 0; i < numRows; i++) {
        for (size_t j = 0; j < numCols; j++) {

            outFile << results_matrix[i][j] << "\t";

            if (j == (numCols-1)) {
                outFile << "\n";
            }

        }

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
int main() {

    //Load instance of SingleCell
    std::unique_ptr<SingleCell> singleCell = std::make_unique<SingleCell>();

    std::vector<std::vector<double>> results_matrix = singleCell->simulate({}, {});

    save_results(results_matrix);

    return 0;
}