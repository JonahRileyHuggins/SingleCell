#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
script name: MPI_Organizer.py
Created on Fri Jan. 10th 12:00:00 2024
Author: Jonah R. Huggins

Description: Curates and handles the organization of tasks for the MPI processes

Output: MPI tasks for each rank, MPI task assignment, and MPI results aggregation
        results storage.

"""
# -----------------------Package Import & Defined Arguements-------------------#
import os
import gc
import logging
import importlib.util

import numpy as np
import pandas as pd
import multiprocessing as mp

from singlecell.Experiment.Manager import Manager
from singlecell.Experiment.ResultsCacher import ResultCache

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
SingleCell = pySingleCell.SingleCell

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

class Worker:

    def __init__(
            self, 
            task: str, 
            sbml_list: list, 
            manager: Manager
            ):

        # Store Manager: methods for job indexing plus results dict
        self.manager = manager

        self.sbmls = sbml_list

        # Store instance of cell
        self.single_cell = None
        
        # Run individual simulation
        self.__run_task(task)

    def __run_task(self, task: str) -> dict:
            """organized simulation method, executed by each process"""
            rank = mp.current_process().name
            if task is None:
                logger.debug(f"Rank {rank} has no tasks to complete")

                return # No need to save anything if no simulation task

            condition, cell, condition_id = self.manager.condition_cell_id(
                rank_task=task,
                conditions_df=self.manager.problem.condition_files[0]
            )

            logger.info(f"{rank} running {condition_id} for cell {cell}")

            self.single_cell = SingleCell(*self.sbmls)

            state_ids = self.single_cell.getGlobalSpeciesIds()

            precondition_results = self.__extract_preequilibration_results(condition_id, cell)
            if precondition_results:
                self.__setModelState(state_ids, precondition_results)

            self.__setModelState(condition.keys(), condition.values.tolist())

            stop_time = self.__get_simulation_time(condition)
            results_array = self.single_cell.simulate(0.0, stop_time, 30.0)

            results = pd.DataFrame(results_array, columns=state_ids)
            results['time'] = np.arange(0, stop_time, 30)

            parcel = self.__package_results(results, condition_id, cell)

            logger.info(f"{rank} finished {condition_id} for cell {cell}")

            self.__cache_results(parcel)

            # Reset rank internal model after simulation and take out the garbage
            self.single_cell = None
            gc.collect()

            logger.info(f"Rank {rank} has completed {condition_id} for cell {cell}")

    def __extract_preequilibration_results(
            self, 
            condition_id: str, 
            cell: int
            ) -> list:
        """
        Find if a given condition has a preequilibration. Pulls from results dictionary
        final timepoint array.
        """
    
        # For now, only supporting one problem per file
        problem = self.manager.problem
        measurement_df = problem.measurement_files[0]
        precondition_results = []

        if 'preequilibrationConditionId' in measurement_df.columns:
            # Filter matching simulationConditionId
            precondition_matches = measurement_df[
                measurement_df['simulationConditionId'] == condition_id
            ]

            if not precondition_matches.empty:
                # Use iloc[0] to safely get the first preequilibrationConditionId
                precondition_id = precondition_matches['preequilibrationConditionId'].iloc[0]
                
                if pd.notna(precondition_id) and str(precondition_id).strip().lower() != 'nan':

                    logger.debug((
                        f"Extracting preequilibration condition {precondition_id}",
                        f"for condition {condition_id}"
                    ))

                    precondition_df = self.manager.results_lookup(precondition_id, cell)
                    
                    if precondition_df is not None:
                        if "time" in precondition_df.columns: 
                            precondition_df = precondition_df.drop("time", axis = 1)

                        precondition_results = precondition_df.iloc[:, -1]

        return precondition_results


    def __setModelState(self, names: list, state: list) -> None:
        """Set model state with list of floats"""
        for index, name in enumerate(names):

            if name in ('conditionId', 'conditionName'):
                continue

            self.single_cell.modify(name, state[index])

        logger.debug("Updated model state")

    def __get_simulation_time(
            self, 
            condition: pd.Series
            ) -> float:
        """
        Returns the simulation time for a condition. Raises an error if time is undefined.
        """
        #Only supporting one problem per config file 
        measurement_df = self.manager.problem.measurement_files[0]
        matching_times = measurement_df.loc[
            measurement_df["simulationConditionId"].isin(condition), "time"
        ]

        if matching_times.empty:
            raise ValueError(
                f"No simulation time defined for condition {condition['conditionId']}"
            )

        return matching_times.max()

    def __cache_results(
            self, 
            parcel: dict
            ) -> None:
        """Saves simulation results to cache directory"""

        condition_id = parcel['conditionId']
        cell = parcel["cell"]
        results = parcel['results']

        for key in self.manager.results_dict.keys(): # <-- ToDo: Not in self

            if self.manager.results_dict[key]['conditionId'] == condition_id \
                and self.manager.results_dict[key]['cell'] == cell:

                # Save results
                cache = ResultCache()
                cache.save(key=key, df=results)

        return # Saves individual simulation data in cache directory

    def __package_results(
            self,
            results: pd.DataFrame,
            condition_id: str,
            cell: str,
        ) -> dict:
        """
        Combines results, condition identifier, and cell number into dict for storage, 
        """

        # make a dict entry in rank_results for every column in results
        rank_results = {
            "conditionId": condition_id,
            "cell": int(cell),
            "results": results,
        }

        return rank_results
