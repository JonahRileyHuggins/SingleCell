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
import sys
import argparse
from types import SimpleNamespace
from typing import Optional
import json

import pandas as pd

sys.path.append("../build/")
from pySingleCell import SingleCell

# Arguement Parsing (Internal For Now)
parser = argparse.ArgumentParser(description='')

parser.add_argument('--modify', '-m', metavar='KEY=VALUE', nargs='+',
                    help='Species to modify in key=value format', default=[])
parser.add_argument('--start', help = 'start time in seconds for simulation', default = 0.0)
parser.add_argument('--stop', help = 'stop time for simulation.', default = 86400.0)
parser.add_argument('--step', help = 'step size of each iteration in the primary for-loop.', default = 30.0)
parser.add_argument('--output', help = 'output path', default="py_simulation_results.tsv")

args = parser.parse_args()

#-----------------------------Static Variables-----------------------------------------#
SBML_DIR = '../sbml_files/'

#-------------------Class Definition-----------------------------------------#
class TestSim:
    """Primary instance of the single cell for simulation."""

    def __init__(self):

        self.stochastic_path = os.path.join(SBML_DIR, 'Stochastic.sbml')

        self.deterministic_path = os.path.join(SBML_DIR, 'Deterministic.sbml')
    
    def simulate(self, args, **kwargs):
        """Primary simulation function using hybrid stochastic-deterministic method

        Parameters:
            - args (simpleNamespace): Namespace taken from command-line arguements.

        Returns: 
            - results_dataframe (pd.DataFrame): finalized results of simulation. 
        """

        # Need to add entity map for deterministic simulations:

        single_cell = SingleCell(self.stochastic_path, self.deterministic_path)

        for pair in args.modify:
            if '=' in pair:
                key, val = pair.split('=', 1)
                print("Setting %s to value %d", key, float(val))
                single_cell.modify(key, float(val))

        results_array = single_cell.simulate(
            args.start,
            args.stop, 
            args.step
            )

        speciesIds = single_cell.getGlobalSpeciesIds()

        results_df = pd.DataFrame(results_array, columns=speciesIds)

        results_df.to_csv(args.output, sep = '\t', index = False)


def parse_dict_arg(arg_string):
    try:
        return json.loads(arg_string)
    except json.JSONDecodeError:
        raise argparse.ArgumentTypeError(f"Invalid JSON format: '{arg_string}'")


if __name__ == '__main__':
    args = parser.parse_args()

    TestSim().simulate(args)
