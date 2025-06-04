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
#include "singlecell/utils.h"

namespace matrix_utils {
    void save_matrix(
        std::vector<std::vector<double>> results_matrix,
            std::string name,
            std::string output,
            std::vector<std::string> row_labels,
            std::vector<std::string> col_labels
    ) {

        // Results path
        std::string outfile = output + name;

        std::ofstream outFile(outfile);

        int numRows = results_matrix.size();

        int numCols = (numRows > 0) ? results_matrix[0].size() : 0;

        if (!col_labels.empty()) {
            outFile << "\t";
            for (const auto& label : col_labels) {
                outFile << label << "\t";
            }
            outFile << "\n";
        }

        for (int i = 0; i < numRows; i++) {
            if (!row_labels.empty()) {
                outFile << row_labels[i] << "\t";
            }
            for (int j = 0; j < numCols; j++) {
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

namespace unit_conversions {
    std::vector<double> nanomolar2mpc(
        std::vector<double> cell_volumes
    ) {

        const double nm2Molar = 0.000000001;

        const double avagadros = 6.022e23;

        std::vector<double> mpc_vec(cell_volumes.size());

        for (int i = 0; i < cell_volumes.size(); i++) {

            mpc_vec[i] = (1.0 / nm2Molar) * (1.0 / cell_volumes[i]) * avagadros;

        }

        return mpc_vec;

    }

    std::vector<double> mpc2nanomolar(
        std::vector<double> cell_volumes
    ) {

        const double avagadros = 6.022e23;

        const double molar2nM = 1.0e9;

        std::vector<double> nanomolar_vec(cell_volumes.size());

        for (int i = 0; i < cell_volumes.size(); i++) {

            nanomolar_vec[i] = (1.0 / avagadros) * (1.0 / cell_volumes[i]) * molar2nM;

        }

        return nanomolar_vec;
    }

    std::vector<double> convert(
        std::vector<double> prior_values,
        std::vector<double> conversion_factors
    ) {

        std::vector<double> converted_vals(prior_values.size());

        for (int i = 0; i < conversion_factors.size(); i++) {

            converted_vals[i] = prior_values[i] * conversion_factors[i];

        }
        return converted_vals;
    }
}