#!/bin/env python3 
"""
filename: Experiment.py
author: Jonah R. Huggins
Created On: 2025-07-10

description: Primary class object of an experiment
"""

import os
import sys
import json
import logging
from concurrent.futures import ProcessPoolExecutor, as_completed

import yaml
import pandas as pd
import pickle as pkl

from shared_utils.file_loader import Config
import shared_utils.utils as utils
import MPI_Organizer as org

sys.path.append("../../../build/")
from pySingleCell import SingleCell

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


class Experiment:

    def __init__(self, petab_yaml: os.PathLike | str, *args, **kwargs) -> None:
        """
        Class object describing a single experiment. 

        Parameters:
        - 
        """

        if self.rank == 0:
            logger.info(f"Starting MPI process across {self.size} number of cores.")

            logger.info("Loading Experiment %s details from %s", (self.name, petab_yaml))

        try:
            petab_yaml = os.path.abspath(petab_yaml)

            assert os.path.exists(petab_yaml)

        except AssertionError:
            raise FileNotFoundError(f"{petab_yaml} is not a valid benchmark")

        # Load the details of the experiment
        self.details = Config.file_loader(petab_yaml)

        self.name = 'None' if self.details.name else self.details.name

        self.communicator, self.rank, self.size = org.mpi_communicator()

        self.loader = org.broadcast_petab_files(
            self.rank, 
            self.communicator, 
            self.yaml_file
        )
        # Pause to allow for the broadcast to complete
        self.communicator.Barrier()

        # Catalogue each rank's list of tasks at root (rank 0)
        # Results dictionary is initialized prior to simulation for convenience
        self.results_dictionary = self.__results_dictionary()


    def run(self) -> dict:
        """
        Primary method to run a Simultion experiment. 

        Returns:
        - results (pickle file | dict): dictionary of results by observation (a.k.a 'Observables')
        """
        # Setup simulation
        
        # Determine the number of rounds and the directory of tasks for each rank
        rounds_to_complete, rank_jobs_directory = org.task_organization(
            self.size,
            self.details
        )

        # For every cell and condition, run the simulation based on the number of rounds
        for round_i in range(rounds_to_complete):

            if self.rank == 0:
                logger.info(f"Round {round_i+1} of {rounds_to_complete}")

            task = org.task_assignment(
                rank=self.rank,
                size=self.size,
                communicator=self.communicator,
                rank_jobs_directory=rank_jobs_directory,
                round_i=round_i
            )

            if task is None:
                logger.debug(f"Rank {self.rank} has no tasks to complete")
                continue

            condition, cell, condition_id = org.condition_cell_id(
                rank_task=task, 
                conditions_df=self.conditions_df
            )

            logger.info(f"Rank {self.rank} is running {condition_id} for cell {cell}")



        return 


    def __extract_preequilibration_results(self, condition_id) -> list:
        """
        Find if a given condition has a preequilibration. Pulls from results dictionary
        final timepoint array. Assigns to 
        """

        

    def __results_dictionary(self):
        """Create an empty dictionary for storing results
        input:
            filtered_conditions: pd.DataFrame - filtered conditions dataframe

        output:
            returns the empty results dictionary, ready to be filled
        """

        #for now, only supporting one problem per file
        problem = self.loader.problems[0].condition_files[0]
        conditions_df = problem.condition_files[0]
        measurement_df = problem.measurement_files[0]


        results = {}

        for condition in conditions_df:

            condition_id = condition["conditionId"]
            num_cells = self.details.cell_count if self.details.cell_count else 1

            for cell in range(num_cells):
                if "datasetId" in measurement_df.columns:
                    identifier = measurement_df["datasetId"]\
                        [measurement_df["simulationConditionId"] == condition_id].values[0]
                else:
                    identifier = utils.identifier_generator()

                results[identifier] = {
                    "conditionId": condition_id,
                    "cell": cell
                }

        return results