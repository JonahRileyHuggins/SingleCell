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
#include <string>


public

        std::vector<std::vector<double>> simulate(std::vector<double> deterministic_initial_states, 
                                                    std::vector<double> stochastic_initial_states,
                                                    double start = 0.0, 
                                                    double stop = 86400.0, 
                                                    double step = 30.0) {
            /**
             * @brief public method for users to interface with the SingleCell Simulator. 
             * 
             * @param deterministic_initial_states are the initial species values for the deterministic AMICI model
             * @param stochastic_initial_states are the initial species values for the stochastic SBML model
             * @param start is the simulation start time
             * @param stop is the simulation stop time, in seconds
             * @param step is the delta_t step between simulation updates in seconds
             * 
             * @returns matrix of global states for both models
             */

            //--ODE--AMICI BLOCK
            std::vector<double> timesteps = setTimeSteps(start, stop, step);

            // Species array to assign results to:
            std::vector<std::vector<double>> results_array(timesteps.size(), std::vector<double>(species_ids.size()));

            //assign initial states as first set of values in results_array
            for (size_t i = 0; i < initial_states.size(); ++i) {
                results_array[0][i] = initial_states[i];
            }

            // Main iterating for-loop: we're going to stop it and update vals every second until total time reached.
            for (int i = 0; i < timesteps.size(); i++) {

                std::vector<double> step_forward = {0, step};
                ode_model->setTimepoints(step_forward);

                //set initial states based on last iteration's final values:
                std::vector<double> current_states = results_array[(i > 0) ? i - 1 : i];

                ode_model->setInitialStates(current_states);

                // Run Simulation
                std::unique_ptr<amici::ReturnData> rdata = amici::runAmiciSimulation(*solver, nullptr, *model);

                //results save step
                std::vector<double> last_vals = getLastValues(*rdata);

                // Implement SGEmodule:
                std::vector<double> stochastic_values = stochastic_module(last_vals);

                for (size_t j = 0; j < stochastic_values.size(); ++j) {

                    results_array[i][j] = stochastic_values[j];

                }

            }

            return results_array;
        }


private:
    // Create Factory Loader method here
    // Maybe a simple function for assigning timepoints per iteration?
    std::vector<double> setTimeSteps(double start, double stop, double step) {
        
        // Initialized array to be returned:
        std::vector<double> timepoints;

        // For loop for calculation
        for (int i = 0; i <= static_cast<int>((stop - start) / step); ++i) {

            timepoints.push_back(static_cast<double>(i));
        }

        // NSteps equivalent in SPARCED:
        return timepoints;
    }

    std::vector<double> setAllSpeciesValues(
                                            std::vector<double> current_states,
                                            std::vector<double> update_states
                                            ) {

        // Creating instance of list to be returned:
        std::vector<double> updated_concentrations;
        for (amici::realtype val : current_states) {

            updated_concentrations.push_back(static_cast<double>(val));

        } 

        // Initial Species modification:
        for (int i = 0; i < update_states.size(); ++i) {

            updated_concentrations[i] = update_states[i];

        }
        
        return updated_concentrations;

    }

    std::vector<double> getLastValues(const amici::ReturnData &rdata) {

        int n_species = rdata.nx; // number of species
        int n_timepoints = rdata.nt; 
        
        const std::vector<double>& all_species = rdata.x; // species trajectories flat array length n_species * n_timepoints

        int last_idx = (n_timepoints-1) * n_species;

        std::vector<double> last_species_values;
        
        // convert each species value from amici::realtype to double
        for (int i = last_idx; i < last_idx + n_species; ++i) {
            last_species_values.push_back(static_cast<double>(all_species[i]));
        
        }

        return last_species_values;
    }

    #endif