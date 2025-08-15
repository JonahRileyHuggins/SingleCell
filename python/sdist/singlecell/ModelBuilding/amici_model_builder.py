#!/usr/bin/env 

"""
filename: amici_model_builder.py
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
import amici
import logging
import subprocess
logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

def amici_builder(
        sbml_path: os.PathLike | str, 
        model_name: str = None,
        output: os.PathLike | str = '../../../../amici_models/', 
        verbose: bool = False
    ):
    """
    Basic AMICI Model compilation step. Takes SBML file as input and outputs AMICI model
    into a binaries directory (`sparced-cpp/bin/AMICI_{MODELNAME}`). 
    """
    
    output_directory = output + model_name
    _make_output_dir(output_directory)

    sbml_importer = amici.SbmlImporter(sbml_path)

    sbml_importer.sbml2amici(model_name, output_directory, verbose = verbose)

    return # Builds AMICI directory in specified path

def _make_output_dir(amici_model_path: str | os.PathLike) -> None:
    """ Provide a path and this returns a directory. Separating from Classes for operability."""
    if not os.path.exists(amici_model_path):
        os.mkdir(path=amici_model_path)

def sanitize_multimodel_build(
    build_dir: os.PathLike | str = "../../../../amici_models/"        
) -> None:
    """
    Removes problematic function in amici CMakeLists build when 2+ AMICI models are present.
    """
    amici_model_count = count_subdirectories(build_dir)

    for item in range(1, amici_model_count):
        item_path = os.path.join(build_dir, item)
        result = subprocess.run([
            "sed", "-i",
            r"/add_custom_target(install-python/,/)/d",
            f"{item_path}/swig/CMakeLists.txt"
            ], 
            capture_output=True,
            text=True,
            check=True
        )

@staticmethod
def count_subdirectories(
    path: os.PathLike | str
) -> int:
    """
    Counts the number of subdirectories within a given directory,. 

    :param path (str):
        path to the directory to inspect

    :return dir_count (int)
        Number of subdirectories found
    """
    if not os.path.isdir(path):
        print(f"Error: '{path}' is not a valid directory.")
        return 0
    
    dir_count = 0

    for item in os.listdir(path):
        item_path = os.path.join(path, item)
        if os.path.isdir(item_path):
            dir_count += 1
    return dir_count


if __name__ == '__main__':

    import argparse
    # Arguement Parsing (Internal For Now)
    parser = argparse.ArgumentParser(description='')

    parser.add_argument('--path', '-p', metavar='SBML_PATH', help='path to an SBML file')
    parser.add_argument('--name', '-n', metavar='model_name', help='[OPTIONAL] User-defined\
                        name for the compiled AMICI model')
    parser.add_argument(
        '-v', '--verbose', 
        action='store_true', 
        help="Enable verbose output.", 
        dest="verbose"
    )
    parser.add_argument(
        '--SBML_Only',
        help='List of solvers to skip AMICI compilation for (SBML-only build)',
        nargs='+',
        default=['stochastic']
    )
    parser.add_argument(
        '--output', '-o',
        help='output directory for model compilation',
        default = '../../../../amici_models/'
    )
    args = parser.parse_args()

    sbml_dir = '../../../../sbml_files/'
    sbml_paths = {
                    'stochastic': sbml_dir+'stochastic.xml', 
                    'deterministic':sbml_dir+'deterministic.xml', 
                    'One4All':sbml_dir+'One4All.xml'
                }

    if args.SBML_Only:

        for index, sbml in enumerate(args.SBML_Only):
            sbml_paths.pop(sbml, None)
    
    for solver, path in sbml_paths.items():
        logger.info("Compiling AMICI model '%s'", solver)
        amici_builder(
            sbml_path=path, 
            model_name=solver, 
            output=args.output, 
            verbose=args.verbose
        )

        # Remove add_custom_target() function for 2+ AMICI models
        sanitize_multimodel_build()
