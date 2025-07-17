#!/bin/env python3 
"""
filename: Experiment.py
author: Jonah R. Huggins
Created On: 2025-07-10

description: Primary class object of an experiment
"""

import os
import sys
import logging
import argparse

import numpy as np
import pandas as pd
import pickle as pkl
from datetime import date

sys.path.append("../")

from shared_utils.file_loader import Config
import shared_utils.utils as utils
import MPI_Organizer as org
import ObservableCalculator as obs
from Visualizer import Visualizer

sys.path.append("../../build/")
from pySingleCell import SingleCell


parser = argparse.ArgumentParser(prog='ModelsCreator')
parser.add_argument('--yaml_path', '-p', default = None, help = 'path to configuration file detailing \
                                                                        which files to inspect for name changes.')
parser.add_argument('--name', '-n', default = 'Deterministic', help = "String-type name of model")
parser.add_argument('--catchall', '-c', metavar='KEY=VALUE', nargs='*',
                    help="Catch-all arguments passed as key=value pairs")
parser.add_argument('-v', '--verbose', help="Be verbose", action="store_true", dest="verbose")
parser.add_argument('--output', '-o', default = ".", help  = "path to which you want output files stored")
parser.add_argument(
    '--observables',
    action='store_false',
    help='Enable downsampling of data'
)


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

        try:
            self.petab_yaml = os.path.abspath(petab_yaml)

            assert os.path.exists(self.petab_yaml)

        except AssertionError:
            raise FileNotFoundError(f"{self.petab_yaml} is not a valid benchmark")

        # Load the details of the experiment
        self.details = Config.file_loader(self.petab_yaml)

        self.name = None if self.details.problems[0].name else self.details.problems[0].name

        self.cell_count = 1 if "cell_count" not in self.details.problems[0] \
            else self.details.problems[0].cell_count

        self.communicator, self.rank, self.size = org.mpi_communicator()

        if self.rank == 0:
            logger.info(f"Starting MPI process across {self.size} number of cores.")

            logger.info("Loading Experiment %s details from %s", self.name, self.petab_yaml)

        # !DotDict Notation! Loader contains configuration file and PEtab files.
        self.loader = org.broadcast_files(
            self.rank, 
            self.communicator, 
            self.petab_yaml
        )

        # SingleCell() constructor is variadic, supply multiple SBML files!
        sbml_list = self.__sbml_getter()

        # Load an instance of SingleCell on every core
        self.single_cell = SingleCell(*sbml_list)

        # Pause to allow for the broadcast to complete
        self.communicator.Barrier()

        # Catalogue each rank's list of tasks at root (rank 0)
        # Results dictionary is initialized prior to simulation for convenience
        if self.rank == 0:
            self.results_dict = self.__results_dictionary()
        else:
            self.results_dict = None

        self.results_dict = self.communicator.bcast(self.results_dict, root=0)


    def run(self) -> None:
        """
        Primary method to run a Simultion experiment. 

        Returns:
        - results (pickle file | dict): dictionary of results by observation (a.k.a 'Observables')
        """

        # Determine the number of rounds and the directory of tasks for each rank
        rounds_to_complete, rank_jobs_directory = org.task_organization(
            self.size,
            self.loader.problems[0].measurement_files[0],
            self.cell_count
        )

        # Get total number of simulation tasks
        total_jobs = len(org.total_tasks(self.loader.problems[0].measurement_files[0], self.cell_count))

        # For every cell and condition, run the simulation based on the number of rounds
        for round_i in range(rounds_to_complete):

            if self.rank == 0:
                logger.info(f"Round {round_i+1} of {rounds_to_complete}")

            # Ensure every rank has the most recent copy of the results dict, to lookup dependency results:
            self.results_dict = self.communicator.bcast(self.results_dict, root=0)

            # Lines 133:139 Assign every rank its task for the current round. 
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
                conditions_df=self.loader.problems[0].condition_files[0]
            )

            logger.info(f"Rank {self.rank} is running {condition_id} for cell {cell}")
            
            # Get species identifiers
            state_ids = self.single_cell.getGlobalSpeciesIds()

            # ---VVV--- Excessive print for big models, uncomment at your own risk!
            # logger.debug(f"List of species names: {state_ids}") 

            # Get results of any preequilibration condition:
            precondition_results = self.__extract_preequilibration_results(condition_id)

            if precondition_results:
                # Assign preequilibration final results to current model state.
                self.__setModelState(state_ids, precondition_results)

            # Assign current condition to model state
            self.__setModelState(condition.keys(), condition)

            # Extract simulation time from condition in measurement_df
            stop_time = self.__get_simulation_time(condition)

            # run simulation
            results_array = self.single_cell.simulate(0.0, stop_time, 30.0) # <-- default start and step times

            results = pd.DataFrame(results_array, columns=state_ids)
            results['time'] = np.arange(0, stop_time, 30)

            # Results are packaged into a single object to reduce the number of items sent via MPI
            parcel = org.package_results(
                results=results,
                condition_id=condition_id,
                cell=cell,
            )

            if self.rank == 0:
                # Store rank 0's results prior to storing other ranks
                self.results_dict = org.store_results(
                    results_dict=self.results_dict, 
                    individual_parcel=parcel
                )

                # Collect results from other ranks and store in results dictionary
                self.results_dict = org.aggregate_other_rank_results(
                    size=self.size,
                    communicator=self.communicator,
                    results_dict=self.results_dict,
                    round_i=round_i,
                    total_jobs=total_jobs,
                )
            else:
                # All non-root ranks send results to rank 0
                self.communicator.send(parcel, dest=0, tag=round_i)
                print(f'Sending rank {self.rank} results to root')

            logger.info(f"Rank {self.rank} has completed {condition_id} for cell {cell}")

        return #Stores results dictionary in class object.


    def __extract_preequilibration_results(self, condition_id) -> list:
        """
        Find if a given condition has a preequilibration. Pulls from results dictionary
        final timepoint array. Assigns to 
        """

        # For now, only supporting one problem per file
        measurement_df = self.loader.problems[0].measurement_files[0]

        precondition_results = []
        
        if 'preequilibrationConditionId' in measurement_df.columns:
            # match condition to any preequilibration:
            precondition_id = measurement_df['preequilibrationConditionId'][
                measurement_df['simulationConditionId'] == condition_id
                ][0]
            logger.debug(f"Extracting preequilibration condition {precondition_id} \
                        for condition {condition_id}")

            # iterate over results_dict to find last results
            precondition_results = self.__results_lookup(precondition_id)

        return precondition_results

    def __results_dictionary(self) -> dict:
        """Create an empty dictionary for storing results
        input:
            filtered_conditions: pd.DataFrame - filtered conditions dataframe

        output:
            returns the empty results dictionary, ready to be filled
        """

        #for now, only supporting one problem per file
        problem = self.loader.problems[0]
        conditions_df = problem.condition_files[0]
        measurement_df = problem.measurement_files[0]

        results = {}

        for idx, condition in conditions_df.iterrows():

            condition_id = condition["conditionId"]
            num_cells = self.details.cell_count if self.details.cell_count else 1

            for cell in range(1, num_cells+1):
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
    

    def __results_lookup(self, condition_id: str) -> list:
        """Indexes results dictionary on condition id, returns last array of condition results"""

        # final results list to store individual species results in:
        final_results = []

        # list for excluding non-species keys in results_dict:
        nonspecies_keys = ['conditionId', 'cell']

        # results keys should all be species names paired with single numpy arrays. 
        for key in self.results_dict.keys():
            if self.results_dict[key]['conditionId'] == condition_id:

                for species in self.results_dict[key].keys():
                    if species not in nonspecies_keys:
                        final_results.append(self.results_dict[key][species][-1])

            return final_results

    def __sbml_getter(self) -> list:
        """Retrieves all sbml files defined in PEtab configuration file"""
        sbml_file_list = [
            fp
            for problem in self.loader.problems
            if hasattr(problem, "sbml_files")
            for fp in problem.sbml_files
        ]

        return sbml_file_list

    def __setModelState(self, names: list, state: list) -> None:
        """Set model state with list of floats"""
        for index, name in enumerate(names):

            if name in ('conditionId', 'conditionName'):
                continue

            self.single_cell.modify(name, state.iloc[index])

        logger.debug("Updated model state")

    def __get_simulation_time(self, condition: pd.Series) -> float:
        """
        Returns the simulation time for a condition. Raises an error if time is undefined.
        """
        #Only supporting one problem per config file at the moment
        measurement_df = self.loader.problems[0].measurement_files[0]
        matching_times = measurement_df.loc[
            measurement_df["simulationConditionId"].isin(condition), "time"
        ]

        if matching_times.empty:
            raise ValueError(
                f"No simulation time defined for condition {condition['conditionId']}"
            )

        return matching_times.max()


    def save_results(self, args) -> None:
        """Save the results of the simulation to a file
        input:
            None
        output:
            returns the saved results as a nested dictionary within
            a pickle file
        """

        # Benchmark results are stored within the specified model directory

        results_directory = os.path.join(os.path.dirname(self.petab_yaml), "results")

        if 'output' in args:

            results_directory = args.output

        if not os.path.exists(results_directory):
            os.makedirs(results_directory)

        # Final output is saved in pickle format
        results_path = os.path.join(results_directory, f"{date.today()}.pkl")

        if self.name is not None:
            results_path = os.path.join(results_directory, f"{self.name}.pkl")


        with open(results_path, "wb") as f:
            pkl.dump(self.results_dict, f)


    def observable_calculation(self, *args) -> None:
        """Calculate the observables and compare to the experimental data.
        input:
            results: dict - results of the SPARCED model unit test simulation
        output:
            returns the results of the SPARCED model unit test simulation
        """
        self.results_dict = obs.ObservableCalculator(self).run()

        self.save_results(args)

        return # Proceeds to next command provided in launchers.py




if __name__ == '__main__':

    args = parser.parse_args()
   
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    experiment = Experiment(args.yaml_path)

    experiment.run()

    logger.debug("Closed simulation method successfully.")

    if experiment.rank == 0 and args.observables == False:
        experiment.save_results(args)
        logger.debug("Saved Results successfully.")

    elif experiment.rank == 0:
        experiment.observable_calculation(args)
        logger.debug("Ran observableCalc. methods successfully")

    
