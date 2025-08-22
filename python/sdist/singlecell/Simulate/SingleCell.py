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
import importlib.util
import argparse
import json

import pandas as pd
from shared_utils.interface import AbstractSimulator
# Absolute path to compiled extension (pySingleCell*.so file)
so_path = os.path.join(
    os.getenv("SINGLECELL_PATH"),
    "build",
    "pySingleCell.cpython-312-x86_64-linux-gnu.so"
)

if not os.path.isfile(so_path):
    raise FileNotFoundError(f"Could not find pySingleCell shared object at: {so_path}")

# Load the module from the given path
spec = importlib.util.spec_from_file_location("pySingleCell", so_path)
pySingleCell = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pySingleCell)

# Access class symbols
SC = pySingleCell.SingleCell

# Arguement Parsing (Internal For Now)
parser = argparse.ArgumentParser(description='Basic script for running single simulations with the SPARCED model')
parser.add_argument('--sbml', '-s', help='SBMLs to be simulated.', nargs='+', default=['../../sbml_files/One4All.xml'])
parser.add_argument('--modify', '-m', metavar='KEY=VALUE', nargs='+',
                    help='Species to modify in key=value format', default=[])
parser.add_argument('--start', help = 'start time in seconds for simulation', default = 0.0)
parser.add_argument('--stop', help = 'stop time for simulation.', default = 86400.0)
parser.add_argument('--step', help = 'step size of each iteration in the primary for-loop.', default = 30.0)
parser.add_argument('--output', help = 'output path', default="singlecell_results.tsv")

#-------------------Class Definition-----------------------------------------#
class SingleCell(AbstractSimulator):
    """Primary instance of the single cell for simulation."""

    def __init__(self, args):
        tool = SC(*args.sbml)
        super().__init__(tool)
        # self.single_cell = SC(*args.sbml)

        self.output = args.output

    
    def simulate(self, start, stop, step):
        """Primary simulation function using hybrid stochastic-deterministic method

        Parameters:

        Returns: 
            - results_dataframe (pd.DataFrame): finalized results of simulation. 
        """

        for pair in self.modify:
            if '=' in pair:
                key, val = pair.split('=', 1)
                print("Setting %s to value %d", key, float(val))
                self.single_cell.modify(key, float(val))

        results_array = self.tool.simulate(
            start,
            stop, 
            step
            )

        speciesIds = self.tool.getGlobalSpeciesIds()

        results_df = pd.DataFrame(results_array, columns=speciesIds)

        results_df.to_csv(self.output, sep = '\t', index = False)

    def modify(self, component, value):
        """
        Method for SingleCell simulator modify method
        """
        self.tool.modify(component, value)



if __name__ == '__main__':
    args = parser.parse_args()

    SingleCell(args).simulate()
