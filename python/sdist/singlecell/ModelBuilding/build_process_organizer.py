#!/bin/env python3 
# -*- coding: utf-8 -*-
"""
Author: JRH
Created: 05/24/2025
Description: Entrypoint to call atomized commands for build:
 1. Antimony file writing
 2. SBML Model generation
 3. AMICI Model compilation
 4. SingleCell source code compilation
"""
import os
import copy
import logging
from typing import TextIO
from types import SimpleNamespace
from collections import defaultdict

from singlecell.shared_utils.file_loader import FileLoader
from singlecell.ModelBuilding.antimony_model_builder import CreateAntimonyFile
from singlecell.ModelBuilding.sbml_model_builder import CreateSBMLModel
from singlecell.ModelBuilding.amici_model_builder import CreateAMICIModel
from singlecell.ModelBuilding.singlecell_builder import build_singlecell

import pandas as pd

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


class Build_Organizer:
    """
    Entrypoint to call atomized commands for build:
        1. Antimony file writing
        2. SBML Model generation
        3. AMICI Model compilation
        4. SingleCell source code compilation
    """

    def __init__(self, args, **kwargs):

        logger.info('Starting build process for solver %s ...', args.name)

        self.antimony_output_dir = args.ANTIMONY_OUTPUT_DIR
        self.sbml_output_dir = args.SBML_OUTPUT_DIR
        self.amici_output_dir = args.AMICI_OUPUT_DIR
        self.singlecell_cmake_source_dir = args.SINGLECELL_CMAKE_SOURCE_DIR
        self.singlecell_build_dir = args.SINGLECELL_BUILD_DIR

        loader = FileLoader(args.path)
        self.model_files = loader._extract_model_build_files()

        # Get all unique solvers:
        self.solvers = self.__get_solvers_list()

        if args.one4all:
            self.solvers.append('One4All')


    def __get_solvers_list(self) -> list:
        """Finds list of unique solver types in species build file"""
        # A) Extract the column & clean / sanitize
        solvers_col = self.model_files.species['solver'].str.lower().str.strip()
        # B) Drop any duplicate solver instances
        unique_solvers = solvers_col.drop_duplicates()
        # C) convert to list
        unique_solvers_list = unique_solvers.tolist()

        return unique_solvers_list


    def __get_components(self, solver: str) -> SimpleNamespace:
        """
        Extract components with attribute `solver` matching input parameter solver

        Parameters:
        - solver (str): 
            name of the solver type specified in the build files
        """

        solver_components = copy.deepcopy(self.model_files)

        if solver == 'One4All':
            solver_components.other_params = pd.DataFrame(
                [], 
                columns=['parameterId', 
                         'value']
            )

        else: 
            # Filter species for non-solver
            solver_components.other_params = self.__get_other_params(
                solver, 
                solver_components.species
            )

            logger.debug('>>>>>>> immediate parameters dataframe: %s' % (solver_components.other_params))

            solver_components.species = solver_components.species[
                solver_components.species['solver'].str.lower().str.strip() == solver
            ]
            print('model species: ',solver_components.species)

        return solver_components


    def __get_other_params(
            self, 
            solver: str,
            species_dataframe: pd.DataFrame) -> pd.DataFrame:
        """Retrieves all species not pertaining to current solver"""

        other_species = species_dataframe[
            species_dataframe['solver'].str.lower().str.strip() != solver
        ].reset_index()

        # Create new DataFrame with desired columns
        other_params = other_species[['speciesId', 'initialConcentration (nM)']].rename(
                columns={'speciesId': 'parameterId', 'initialConcentration (nM)': 'value'}
            )
        
        return other_params


    def __get_row_rcts_pdts(self, row: pd.Series) -> list:
        """
        Extracts the reactants and products for a particular reaction

        Params:
        - row (pandas.Series):
            Ratelaws dataframe row iterative containing column 'r ; p'

        Returns:
        - reaction_parts (list): list of all reactants and products within Ratelaw_df.index[row]
        """

        reaction_parts = []

        rxn = row.get('r ; p')

        if pd.isna(rxn) or not isinstance(rxn, str):
            logger.debug("Reaction entry is NaN or not a string: %s", rxn)
            return reaction_parts

        reactants_str, products_str = (rxn.split(';') + [''])[:2]
        logger.debug("Parsed reaction string: reactants='%s', products='%s'", reactants_str, products_str)

        reactants = [r.strip() for r in reactants_str.split('+') if r.strip()]
        products = [p.strip() for p in products_str.split('+') if p.strip()]

        reaction_parts = reactants + products

        return reaction_parts


    def __reduce_rxns(
            self,
            solver_components: pd.DataFrame
        ) -> pd.DataFrame:
        """
        excludes ratelaws containing components not of type `solver`. 
        Deciding method by whether species is (reactant | product) in current model.

        Parameters
        - solver_components (pd.DataFrame): 
            Model build dataframes of solver-specific data

        Returns
        - solver_components (pd.DataFrame):
            Model build dataframes of solver-specific data, with `solver`-only ratelaws
        """

        # Collector list for species to drop
        drop_indices = []

        model_speciesIds = set(solver_components.species.index.to_list())

        for reactionId, row in solver_components.ratelaws.iterrows():

            reaction_parts = self.__get_row_rcts_pdts(row=row)

            for species in reaction_parts:
                if species not in set(model_speciesIds):

                    logger.debug("Excluding non-model reaction %s: %s", reactionId, species)
                    drop_indices.append(reactionId)
                    break  # no need to check more species for this reaction

        solver_components.ratelaws.drop(index=drop_indices, inplace=True)

        return solver_components


    def build_antimony_files(self) -> None:
        """
        calls class CreateAntimonyFile to write model build files into antimony files
        for all solvers in model files

        Parameters:
        - None

        Returns:
        - None 
            Saves Antimony files to disk at args.ANTIMONY_OUTPUT_DIR
        """

        for solver in self.solvers:
            
            solver_components = self.__get_components(solver)

            solver_components = self.__reduce_rxns(solver_components)

            # Composition method, not the cleanest but works.
            CreateAntimonyFile(
                solver_components,
                solver,
                self.antimony_output_dir
                )


    def build_sbml_models(self) -> None:
        """
        calls class CreateSBMLModels to write antimony models into SBML models
        for all solvers in model files

        Parameters:
        - None

        Returns:
        - None 
            Saves SBML models to disc at args.SBML_OUTPUT_PATH
        """

        for solver in self.solvers:
            
            solver_components = self.__get_components(solver)

            solver_components = self.__reduce_rxns(solver_components)

            # Composition method, not the cleanest but works.
            CreateSBMLModel(
                solver, 
                solver_components,
                self.antimony_output_dir, 
                self.sbml_output_dir
                )


    def build_amici_models(self) -> None:
        """
        calls class CreateSBMLModels to write antimony models into SBML models
        for all solvers in model files

        Parameters:
        - None

        Returns:
        - None 
            Saves SBML models to disc at args.SBML_OUTPUT_PATH
        """

        for solver in self.solvers:

            sbml_path = os.path.join(self.sbml_output_dir, f"{solver}.xml")

            # Composition method, not the cleanest but works.
            logger.info("Compiling AMICI model '%s'", solver)
            CreateAMICIModel(
                sbml_path=sbml_path, 
                model_name=solver, 
                output=self.amici_output_dir, 
                verbose=self.verbose
            )

        # Remove add_custom_target() function for 2+ AMICI models
        CreateAMICIModel.sanitize_multimodel_build(self.amici_output_dir)


    def build_singlecell_code(self) -> None:
        """
        calls functionn build_singlecell to configure, build, and install 
        CMake files.

        Parameters:
        - None

        Returns:
        - None 
            Compiles source code at args.SINGLECELL_BUILD_DIR
        """

        build_singlecell(self.singlecell_build_dir, self.singlecell_cmake_source_dir)

