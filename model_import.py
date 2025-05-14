#!/usr/bin/env 

"""
filename: model_import.py
created by: Jonah R. Huggins
created on: 25-04-28

description: 
Amici requires the python module to be used to compile a model from an SBML file. 
This file is intending to use the AMICI v0.31.1 API to compile said model from an 
incredibly basic "Ligand-Receptor Model". Then, the C++ exposed points will be used
to manipulate the dataset and pass data back and forth.
"""
# Package Imports
import os 
import argparse

import amici


# Arguement Parsing (Internal For Now)
parser = argparse.ArgumentParser(description='')

parser.add_argument('--SBML_PATH', '-p', metavar='SBML_PATH', help='path to an SBML file')
parser.add_argument('--model_name', '-n', metavar='model_name', help='[OPTIONAL] User-defined\
                    name for the compiled AMICI model')

args = parser.parse_args()

def main(sbml_path: os.PathLike | str, model_name: str = None):
    """
    Basic AMICI Model compilation step. Takes SBML file as input and outputs AMICI model
    into a binaries directory (`sparced-cpp/bin/AMICI_{MODELNAME}`). 
    """

    output_directory = f'amici_models/{model_name}' # Hard coded path to bin file for now. Will probably move this to src folder eventually

    if not os.path.exists(output_directory):
        os.mkdir(output_directory)


    sbml_importer = amici.SbmlImporter(sbml_path)

    sbml_importer.sbml2amici(model_name, output_directory, verbose = True)

    return 

if __name__ == '__main__':
    main(args.SBML_PATH, args.model_name)