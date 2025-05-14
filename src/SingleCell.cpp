// SingleCell.cpp
#include "SingleCell.h"

std::vector<double> SingleCell::setTimeSteps(double start, double stop, double step) {
    std::vector<double> timesteps;
    for (double t = start; t <= stop; t += step) {
        timesteps.push_back(t);
    }
    return timesteps;
}

std::vector<double> SingleCell::setAllSpeciesValues(const std::vector<double>& current_states,
                                                    const std::vector<double>& update_states) {
    // Combine deterministic and stochastic state vectors
    std::vector<double> all_states = current_states;
    all_states.insert(all_states.end(), update_states.begin(), update_states.end());
    return all_states;
}

std::vector<double> SingleCell::getLastValues(const amici::ReturnData& rdata) {
    // Grab the last state values from AMICI output
    return rdata.x[rdata.nt - 1];
}
