/**
 * @file Simulation.cpp
 * 
 * @brief Combines instances of Deterministic and Stochastic Modules into singular simulation framework
 * 
 * @authors Jonah R. Huggins, Marc R. Birtwistle
 * @date 15-05-2025
 */

// --------------------------Library Import--------------------------------//
#include <vector>
#include <string>
#include <unordered_set>


// Internal Libraries
#include "singlecell/Simulation.h"
//-----------------------------Class Details-------------------------------//
Simulation::Simulation(
    std::string stochastic_sbml_path,
    std::string deterministic_sbml_path
)

{

}

Simulation::~Simulation() {

}

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

    std::vector<std::vector<double>> results_matrix(numTimeSteps, std::vector<double>(numSpecies));

    return results_matrix;

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

    combined_matrix.insert(combined_matrix.end(), matrix2.begin(), matrix2.end());

    return combined_matrix;
}
