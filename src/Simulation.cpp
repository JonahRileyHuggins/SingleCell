/**
 * @file Simulation.cpp
 * 
 * @brief Combines instances of Deterministic and Stochastic Modules into singular simulation framework
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 19-05-2025
 */

//===========================Library Import=================================//
//Std Libraries
#include <vector>
#include <string>
#include <memory>
#include <unordered_set>


// Internal Libraries
#include "singlecell/Simulation.h"
#include "singlecell/SBMLHandler.h"
//=============================Class Details================================//
Simulation::Simulation(
    SBMLHandler Module
) : handler(Module) {}

std::vector<double> Simulation::setTimeSteps(double start, double stop, double step) {
     // Initialized array to be returned:
    std::vector<double> timepoints;

    // For loop for calculation
    for (int i = 0; i <= static_cast<int>((stop - start) / step); ++i) {

        timepoints.push_back(static_cast<double>(i));
    }

    // NSteps equivalent in SPARCED:
    return timepoints;
}

std::vector<std::vector<double>> Simulation::createResultsMatrix(
    int numSpecies,
    int numTimeSteps
) {

    std::vector<std::vector<double>> results_matrix(numSpecies, std::vector<double>(numTimeSteps));

    printf("Matrix @ creation: dimensions: [%lu, %lu]", results_matrix.size(), results_matrix[0].size());
    printf("\n");

    return results_matrix;

}

void Simulation::recordStepResult(
    const std::vector<double>& state_vector,
    int timepoint
) {
    for (int i = 0; i < state_vector.size(); i++) {
        results_matrix[i][timepoint] = state_vector[i];
    }
}

std::vector<std::string> Simulation::findOverlappingIds(
    const std::vector<std::string>& ids1,
    const std::vector<std::string>& ids2
) {
    std::vector<std::string> overlaps;
    std::unordered_set<std::string> lookup(ids2.begin(), ids2.end());

    for (const auto& id : ids1) {
        if (lookup.count(id)) {
            overlaps.push_back(id);
        }
    }

    return overlaps;
}

std::vector<std::vector<double>> Simulation::concatenateMatrixRows(
    std::vector<std::vector<double>> matrix1, 
    std::vector<std::vector<double>> matrix2
) {
    std::vector<std::vector<double>> combined_matrix = matrix1;

    printf("Matrix 1 dimensions: [%lu, %lu]", matrix1.size(), matrix1[0].size());
    printf("\n");
    printf("Matrix 2 dimensions: [%lu, %lu]", matrix2.size(), matrix2[0].size());
    printf("\n");

    combined_matrix.insert(combined_matrix.end(), matrix2.begin(), matrix2.end());

    return combined_matrix;
}
