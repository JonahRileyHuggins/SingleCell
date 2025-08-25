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
from pathlib import Path

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

base_path = Path(os.environ.get("SINGLECELL_PATH", Path.home() / ".local/share/SingleCell"))
class CreateAMICIModel:

    def __init__(
            self,
            sbml_path: os.PathLike | str, 
            model_name: str = None,
            output: os.PathLike | str = base_path / "amici_models", 
            verbose: bool = False
        ):
        """
        Basic AMICI Model compilation step. Takes SBML file as input and outputs AMICI model
        into a binaries directory (`sparced-cpp/bin/AMICI_{MODELNAME}`). 
        """
        
        output_directory = str(output) + "/" + model_name
        self._make_output_dir(output_directory)

        sbml_importer = amici.SbmlImporter(sbml_path)

        # AMICI stores an instance of the SBML object inside the importer object.
        sbml_model = sbml_importer.sbml

        constantParams = [params.getId() for params in sbml_model.getListOfParameters()]

        sbml_importer.sbml2amici(
            model_name, 
            output_directory, 
            verbose = verbose, 
            constant_parameters=constantParams
        )


    def _make_output_dir(amici_model_path: str | os.PathLike) -> None:
        """ Provide a path and this returns a directory. Separating from Classes for operability."""
        if not os.path.exists(amici_model_path):
            os.mkdir(path=amici_model_path)

    @staticmethod
    def sanitize_multimodel_build(
        build_dir: os.PathLike | str = base_path / "amici_models"        
    ) -> None:
        """
        Removes problematic function in amici CMakeLists build when 2+ AMICI models are present.
        """
        for item in os.listdir(build_dir):
            item_path = os.path.join(build_dir, item)

            # Skip if it's not a directory
            if not os.path.isdir(item_path):
                continue

            cmake_file = os.path.join(item_path, "swig", "CMakeLists.txt")
            if not os.path.exists(cmake_file):
                continue

            subprocess.run(
                [
                    "sed", "-i",
                    r"/add_custom_target(install-python/,/)/d",
                    cmake_file
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
