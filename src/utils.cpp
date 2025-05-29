/**
 * @file utils.cpp
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 28-05-2025
 * 
 * @brief Details for general, global utility functions
*/
//===========================Library Import=================================//
// Standard Libraries
#include <fstream>
#include <iostream>

// Internal Libraries
#include "singlecell/utils.h"

namespace matrix_utils {
    void save_matrix(
        std::vector<std::vector<double>> results_matrix,
            std::string name,
            std::string output
    ) {

        // Results path
        std::string outfile = output + name;

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

    std::vector<double> getColumn(
        const std::vector<std::vector<double>>& matrix, 
        size_t indexCol
    ) {
        std::vector<double>column;

        for (const auto& row : matrix) {
            column.push_back(row[indexCol]);
        }

        return column;
    }
}
