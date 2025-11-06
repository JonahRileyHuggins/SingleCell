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

#include <Eigen/Dense>

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
    Eigen::VectorXd nanomolar2mpv(
        Eigen::VectorXd cell_volumes
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
    Eigen::VectorXd molecules2nanomolar(
        Eigen::VectorXd cell_volumes
    );

}

#endif