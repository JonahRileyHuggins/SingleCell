#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
script name: simulation.py
Created on Thurs. 04/11/2025 13:00:00
Author: Jonah R. Huggins

Description: File for orchestrating communication between the AMICI simulation function and a custom
            simple math routine for modifying SBML results and passing back.    
Output:
    Simulation Results

"""

# -----------------------Package Import & Defined Arguements-------------------#
# Package Imports
import os 
import argparse

import time
import amici
import numpy as np
import pandas as pd

from SGEmodule import SGEmodule


# Arguement Parsing (Internal For Now)
parser = argparse.ArgumentParser(description='')

parser.add_argument('--model_dir', '-p', metavar='model_dir', help='path to an AMICI model', default="../amici_models/SPARCED")
parser.add_argument('--model_name', '-n', metavar='model_name', help='[OPTIONAL] User-defined\
                    name for the compiled AMICI model', default= "SPARCED")
parser.add_argument('--target', '-t', metavar = 'target', help='species to modify', default = '_L_')
parser.add_argument('--concentration', '-c', help= 'nanomolar concentration\
                     of species to be updated', default= 10.0)
parser.add_argument('--start', help = 'start time in seconds for simulation', default = 0.0)
parser.add_argument('--stop', help = 'stop time for simulation.', default = 60.0)
parser.add_argument('--step', help = 'step size of each iteration in the primary for-loop.', default = 0.001)
parser.add_argument('--output', help = 'output path', default="py_simulation_results.tsv")

def main(args):
    """
    Runs a basic for loop for simulation. 
    """

    model_module = amici.import_model_module(args.model_name, args.model_dir)

    model = model_module.getModel()

    solver = model.getSolver()

    species_ids = model.getStateIds()

    initial_states = list(model.getInitialStates())

    updated_concentrations = setSingleSpeciesValue(species_ids, 
                                                   initial_states,
                                                   args.target, 
                                                   float(args.concentration))

    model.setInitialStates(updated_concentrations)

    timesteps = setTimeSteps(float(args.start), float(args.stop), float(args.step))

    solver.setAbsoluteTolerance(1e-10)
    solver.setRelativeTolerance(1e-10)
    solver.setMaxSteps(10000)

    results_array = np.zeros((len(timesteps), len(species_ids)))

    for i in range(len(updated_concentrations)):
        results_array[0][i] = updated_concentrations[i]

    # Main For-Loop.
    for i in range(len(timesteps)):

        step_forward = [0, float(args.step)]

        model.setTimepoints(step_forward)

        current_states = results_array[i - 1 if (i > 0) else 0]

        model.setInitialStates(current_states)

        rdata = amici.runAmiciSimulation(model, solver)

        # last_vals = getLastValues(rdata)
        last_vals = rdata._swigptr.x[-len(species_ids):]

        ## Implement fake SGE module here
        stochastic_values = SGEmodule(last_vals) # <<< SGEmodule function place
        rdata = None

        for j in range(len(stochastic_values)-1):
            # xoutS_all[qq+1,:] = rdata._swigptr.x[-n_sp:]
            results_array[i][j] = stochastic_values[j]

    save_results(results_array, timesteps, species_ids, args.output)

    return None


def setSingleSpeciesValue(species_ids, initial_states, target, concentration):
    """
    Takes a single species and concentration, updates array of concentrations, 
    returns updated_concentration array
    """
    updated_concentrations = initial_states

    for index, value in enumerate(species_ids):
        if target == value:
            updated_concentrations[index] = concentration

    return updated_concentrations


def setTimeSteps(start: int, stop: int, step:float):
    """
    Basic math operation to make 
    """
    timepoints = np.array([])

    for i in range(int((stop - start ) / step)):
        timepoints= np.append(timepoints, i)

    return timepoints


def getLastValues(rdata: amici.ReturnData):
    """
    Retrieves last values from rdata array.
    """

    n_species = rdata.nx # Number of species
    n_timepoints = rdata.nt # Amici timepoints from start to stop within for loop. 

    all_species = (rdata.x).flatten() # species trajectories

    last_idx = (n_timepoints - 1) * n_species

    last_species_vals = np.array([])

    for i in range(last_idx, (last_idx + n_species)):
        last_species_vals = np.append(last_species_vals, all_species[i])

    return last_species_vals


def save_results(results_array: np.array, 
                 timepoints: np.array, 
                 species_ids: list,
                 output_name: os.PathLike):
    """
    Saves simulation results.
    """

    results_df = pd.DataFrame(results_array, index=timepoints, columns=species_ids)

    results_df.to_csv(output_name, sep = '\t', header=True, index=True)

if __name__ == '__main__':
        
    #Starting For-loop time:
    start_time = time.process_time()

    args = parser.parse_args()
    main(args)


    # saving round results to array:
    end_time = time.process_time()

    cpu_time = end_time - start_time

    with open("../python_total_time.tsv", mode = 'a+') as f:

        f.write(str(cpu_time) + "\n")