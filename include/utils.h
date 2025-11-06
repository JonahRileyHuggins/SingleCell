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