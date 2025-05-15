/*
filename: SingleCell.h
created by: Jonah R. Huggins
created on: 25-05-14

description: 
Class Creator For Single Cell Model. 
*/
//--------------helper function definition------------------------------------//

#ifndef SINGLECELL_h
#define SINGLECELL_h

// --------------------------Library Import-----------------------------------//
#include <vector>
#include <memory>
#include "amici/amici.h"

class SingleCell {
protected:
    std::vector<double> setTimeSteps(double start, double stop, double step);

    std::vector<double> setAllSpeciesValues(const std::vector<double>& current_states,
                                            const std::vector<double>& update_states);

    std::vector<double> getLastValues(const amici::ReturnData& rdata);
    
    virtual void exchangeData(); // Will be overridden in children if needed

public:
    SingleCell();
    virtual ~SingleCell() = default;

    virtual std::vector<std::vector<double>> simulate(
        const std::vector<double>& deterministic_initial_states,
        const std::vector<double>& stochastic_initial_states,
        double start,
        double stop,
        double step
    ) = 0; // pure virtual to enforce implementation in child
};

#endif // SINGLECELL_H
