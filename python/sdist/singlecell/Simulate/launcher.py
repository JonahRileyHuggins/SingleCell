#!/bin/env python3 
"""
Created on 2025-08-22
Author: Jonah R. Huggins

Description: Handles launching simulation with complex arguments,
just here for atomicity and reducing size of CLI/cli.py

Input: Simulation Settings

Output:
    Simulation Results

"""
# -----------------------Package Import & Defined Arguements-------------------#
import sys
from types import SimpleNamespace

import pandas as pd

from SingleCell import SingleCell

def Simulation(args: SimpleNamespace):
    """Main launcher method for single simulation via CLI"""
    
    try: 
            
            single_cell = SingleCell(*args.sbmls)

    except FileNotFoundError:
        print("Invalid sbml path supplied")
        sys.exit(0)

    if 'modify' in args.__dict__.keys():
        for pair in args.modify:
            if '=' in pair:
                key, val = pair.split('=', 1)
                print("Setting %s to value %d", key, float(val))
                single_cell.modify(key, float(val))

    results_df = single_cell.simulate(start=args.start, stop=args.stop, step=args.step)

    output_path = args.output if args.output != '.' else 'results.tsv'

    results_df.to_csv(output_path, sep='\t', index=False)
