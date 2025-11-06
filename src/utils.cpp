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