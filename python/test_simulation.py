#!/bin/env python3 
"""
script name: SingleCell.py
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
from typing import Optional
import json

from _sbml_handler import _SBMLHandler
import SingleCell as scs

import pandas as pd


# Arguement Parsing (Internal For Now)
parser = argparse.ArgumentParser(description='')

parser.add_argument('--target', '-t', metavar = 'target', help='species to modify', default = {})
parser.add_argument('--start', help = 'start time in seconds for simulation', default = 0.0)
parser.add_argument('--stop', help = 'stop time for simulation.', default = 86400.0)
parser.add_argument('--step', help = 'step size of each iteration in the primary for-loop.', default = 30.0)
parser.add_argument('--output', help = 'output path', default="py_simulation_results.tsv")

args = parser.parse_args()

#-----------------------------Static Variables-----------------------------------------#
SBML_DIR = '../sbml_files/'

#-------------------Class Definition-----------------------------------------#
class SingleCell:
    """Primary instance of the single cell for simulation."""

    def __init__(self):

        # A cell needs a Genome!
        self.stochastic_path = os.path.join(SBML_DIR, 'Stochastic.sbml')
        self.stochastic_model = self._load_stochastic_sbml()

        # A Cell also needs proteins!
        self.deterministic_path = os.path.join(SBML_DIR, 'Deterministic.sbml')
        self.deterministic_model = self._load_deterministic_sbml()

    def _load_stochastic_sbml(self) -> Optional[_SBMLHandler]:
        """Loads the stochastic SBML file containing Cell Genome."""

        return _SBMLHandler(self.stochastic_path)
    
    def _load_deterministic_sbml(self) -> Optional[_SBMLHandler]:
        """Loads the Protein-Protein Interactions SBML file."""

        return _SBMLHandler(self.deterministic_path)
    
    
    def simulate(self, args, **kwargs):
        """Primary simulation function using hybrid stochastic-deterministic method

        Parameters:
            - args (simpleNamespace): Namespace taken from command-line arguements.

        Returns: 
            - results_dataframe (pd.DataFrame): finalized results of simulation. 
        """

        ode_sbml = self.deterministic_model.sbml_model

        stochastic_sbml = self.stochastic_model.sbml_model

        deterministic_species_names = [ode_sbml.getSpecies(i).getId() \
                     for i in range(ode_sbml.getNumSpecies())]

        stochastic_species_names = [stochastic_sbml.getSpecies(i).getId() \
                     for i in range(stochastic_sbml.getNumSpecies())]

        initial_ode_states = self.deterministic_model._get_initial_model_states()

        initial_stochastic_states = self.deterministic_model._get_initial_model_states()

        target = parse_dict_arg(args.target)

        updated_ode_states = _SBMLHandler.set_species_values(initial_ode_states, 
                                                             deterministic_species_names, 
                                                             target) # <--- Add in model attribute update function here
        
        updated_stoch_states = _SBMLHandler.set_species_values(initial_stochastic_states, # <--- Add in model attribute update function here
                                                             stochastic_species_names, 
                                                             target) 

        single_cell = scs.SingleCell(self.stochastic_path, self.deterministic_path)

        results_array = single_cell.simulate(updated_ode_states, updated_stoch_states, 
                                     args.start, args.stop, args.step)

        results_df = pd.DataFrame(results_array, columns=deterministic_species_names.extend(stochastic_species_names))

        results_df.to_csv(args.output)


def parse_dict_arg(arg_string):
    try:
        return json.loads(arg_string)
    except json.JSONDecodeError:
        raise argparse.ArgumentTypeError(f"Invalid JSON format: '{arg_string}'")


if __name__ == '__main__':
    args = parser.parse_args()

    SingleCell().simulate(args)
