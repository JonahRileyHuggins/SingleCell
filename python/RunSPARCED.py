#!/bin/env python3 
"""
script name: RunSPARCED.py
Created on Thurs. 2025-04-12 20:29:00
Author: Jonah R. Huggins

Description: [**PROTOTYPE**] entrypoint for the SPARCED model simulation. Parsing and wrapper \n
function that takes model attributes, performs basic data handling, and passes \n
initial model state array to SPARCED C++ simulation engine  

Input: Simulation Settings

Output:
    Simulation Results

"""
# -----------------------Package Import & Defined Arguements-------------------#

import os 
import argparse
from types import SimpleNamespace
import json

import RunSPARCED

import time
import libsbml
import numpy as np
import pandas as pd


# Arguement Parsing (Internal For Now)
parser = argparse.ArgumentParser(description='')

parser.add_argument('--target', '-t', metavar = 'target', help='species to modify', default = {})
parser.add_argument('--start', help = 'start time in seconds for simulation', default = 0.0)
parser.add_argument('--stop', help = 'stop time for simulation.', default = 60.0)
parser.add_argument('--step', help = 'step size of each iteration in the primary for-loop.', default = 0.001)
parser.add_argument('--sbml_path', help = 'Path to sbml file', default = "../sbml_files/sbml_Basic_model.xml")
parser.add_argument('--output', help = 'output path', default="py_simulation_results.tsv")

args = parser.parse_args()
#-------------------Function Definition-----------------------------------------#
def run(args: SimpleNamespace):
    """
    Primary wrapper function for the SPARCED model simulation code. 

    Parameters:
    - args (simpleNamespace): Namespace taken from command-line arguements.

    Returns: 
    - results_dataframe (pd.DataFrame): finalized results of simulation. 
    """

    sbml_model = get_sbml_model(args.sbml_path)

    species_names = [sbml_model.getSpecies(i).getId() \
                     for i in range(sbml_model.getNumSpecies())]

    initial_states = get_initial_model_states(sbml_model)

    target = parse_dict_arg(args.target)

    updated_states = set_species_values(initial_states, 
                                        species_names, 
                                        target)

    # Now we run the primary function:
    results_array = RunSPARCED.run(updated_states,args.start, args.stop, args.step)

    results_df = pd.DataFrame(results_array, columns=species_names)

    results_df.to_csv(args.output)


def parse_dict_arg(arg_string):
    try:
        return json.loads(arg_string)
    except json.JSONDecodeError:
        raise argparse.ArgumentTypeError(f"Invalid JSON format: '{arg_string}'")


def get_sbml_model(sbml_path: os.PathLike):
    # Read-in the model SBML to get compartmental volumes (used to convert nM to mpc and vice versa)
    sbml_reader = libsbml.SBMLReader()
    sbml_doc = sbml_reader.readSBML(sbml_path)

    return sbml_doc.getModel()


def set_species_values(changing_states: list, 
                       species_names: list, 
                       target: dict) -> np.ascontiguousarray:
    """
    Iterates over list of species to be changed, assigns them to an initial state vector

    Parameters:
    - changing_states (list): states recieved that need to be changed within the model class
    - species_names (list): Model-species' names 

    Returns:
    - initial_states (np.ndarray): species-length array of values, updated with user-defined values
    """

    for index, species in enumerate(species_names):
        if species in target:
            changing_states[index] = target[species]

    return changing_states


def get_initial_model_states(model: libsbml.Model):
    """
    function obtains model values
    """

    initial_sbml_states = []

    # Loop through species and get initial concentrations or amounts
    for i in range(model.getNumSpecies()):
        species = model.getSpecies(i)

        if species.isSetInitialConcentration():
            initial_value = species.getInitialConcentration()
        elif species.isSetInitialAmount():
            initial_value = species.getInitialAmount()
        else:
            initial_value = 0.0  
    
        initial_sbml_states.append(initial_value)

    return initial_sbml_states


if __name__ == '__main__':
    args = parser.parse_args()

    start = time.process_time()

    run(args)

    end = time.process_time()

    cpu_time = end - start

    with open("../c++_total_time.tsv", mode = 'a+') as f:

        f.write(str(cpu_time) + "\n")