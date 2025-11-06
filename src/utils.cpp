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
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

// Internal Libraries
#include "utils.h"

// Third Party Libraries
#include <Eigen/Dense>

namespace matrix_utils {
    void save_matrix(
        std::vector<std::vector<double>> results_matrix,
            std::string output,
            std::vector<std::string> row_labels,
            std::vector<std::string> col_labels
    ) {

        std::ofstream outFile(output);

        int numRows = results_matrix.size();

        int numCols = (numRows > 0) ? results_matrix[0].size() : 0;

        if (!col_labels.empty()) {
            outFile << "index";
            for (const auto& label : col_labels) {
                outFile << "\t" << label;
            }
            outFile << "\n";
        }

        for (int i = 0; i < numRows; i++) {
            if (!row_labels.empty()) {
                outFile << row_labels[i];
            }
            for (int j = 0; j < numCols; j++) {
                outFile << "\t" << results_matrix[i][j];

            }

            outFile << "\n";

        }

        outFile.close();

    }

    std::vector<double> getColumn(
        const std::vector<std::vector<double>>& matrix, 
        size_t indexCol
    ) {
        std::vector<double>column(matrix.size());

        for (int r = 0; r < matrix.size(); r++) {

            column[r] = matrix[r][indexCol];

        }

        return column;
    }
}

namespace unit_conversions {
    Eigen::VectorXd nanomolar2mpv(const Eigen::VectorXd& cell_volumes) {
        const double nm2Molar = 1e9;
        const double avogadro = 6.022e23;

        const double conversion = (1.0 / nm2Molar) * avogadro;

        return Eigen::VectorXd::Constant(cell_volumes.size(), conversion);
    }

    Eigen::VectorXd molecules2nanomolar(const Eigen::VectorXd& cell_volumes) {
        const double avogadro = 6.022e23;
        const double molar2nM = 1.0e9;

        // Elementwise calculation: 1 / (cell_volume * avogadro) * molar2nM
        return (1.0 / (cell_volumes.array() * avogadro)) * molar2nM;
    }
}