/**
 * @file utils.h
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 28-05-2025
 * 
 * @brief namespace methods for basic utility functions. Globally applicable
 */
//========================header file definition============================//
#pragma once

#ifndef utils_h
#define utils_h

//===========================Library Import=================================//
#include <vector>
#include <string>
#include <memory>

//=======================Namespace Definition===============================//

namespace matrix_utils {

    /**
     * @brief saves a matrix to a specified file name and location
     * 
     * @param results_matrix the matrix to be saved
     * @param name string name for output file, default "results.tsv"
     * @param output path string to where results matrix should be saved
     * 
     * @returns None
     */
    void save_matrix(
        std::vector<std::vector<double>> results_matrix,
        std::string output,
        std::vector<std::string> row_labels = {},
        std::vector<std::string> col_labels = {}
        );

    /**
     * @brief Extracts a column based on index from a matrix of nested vectors
     * 
     * @param matrix the matrix to extract from
     * @param colIndex the index to extract from said matrix
     * 
     * @returns matrix-column @ colIndex
     */
    std::vector<double> getColumn(
        const std::vector<std::vector<double>>& matrix, 
        size_t colIndex
    );
}

namespace unit_conversions {
    /**
     * @brief conversion factor list for nanomolar to molecules per cell concentration,
     * considering every component's compartmental volume
     * 
     * @param cell_volumes list of compartment volumes for every species
     * 
     * @return mpc_vec list of static conversion factors 
     * from unit  units nanomolar to molecules per cell
     */
    std::vector<double> nanomolar2mpc(
        std::vector<double> cell_volumes
    );

    /**
     * @brief conversion factor list for molecules per cell to nanomolar concentration,
     * considering every component's compartmental volume
     * 
     * @param cell_volumes list of compartment volumes for every species
     * 
     * @return nanomolar_vec list of static conversion factors 
     * from unit molecules per cell to units nanomolar
     */
    std::vector<double> mpc2nanomolar(
        std::vector<double> cell_volumes
    );

    /**
     * @brief method to convert iterational values to proper units per model.convert
     * 
     * @param prior_values list of values before conversion
     * @param conversion_factors list of conversion factors, same length as prior_values
     * 
     * @returns converted_vals element-wise multiplied values per item in prior_values
     */
    std::vector<double> convert(
        std::vector<double> prior_values,
        std::vector<double> conversion_factors
    );
}

#endif