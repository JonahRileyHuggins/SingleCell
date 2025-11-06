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
    inline Eigen::VectorXd nanomolar2mpv(const Eigen::VectorXd& cell_volumes) {
        const double nm2Molar = 1e9;
        const double avogadro = 6.022e23;
        const double conversion = (1.0 / nm2Molar) * avogadro;
        return Eigen::VectorXd::Constant(cell_volumes.size(), conversion);
    }

    /**
     * @brief conversion factor list for molecules per cell to nanomolar concentration,
     * considering every component's compartmental volume
     * 
     * @param cell_volumes list of compartment volumes for every species
     * 
     * @return nanomolar_vec list of static conversion factors 
     * from unit molecules per cell to units nanomolar
     */
    inline Eigen::VectorXd molecules2nanomolar(const Eigen::VectorXd& cell_volumes) {
        const double avogadro = 6.022e23;
        const double molar2nM = 1.0e9;
        return (1.0 / (cell_volumes.array() * avogadro)) * molar2nM;
    }

    }

#endif