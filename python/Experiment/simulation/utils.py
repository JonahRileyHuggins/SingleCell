#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
script name: utils.py
Created on Fri April 24 18:00:00 2024
Author: Jonah R. Huggins

Description: This script is designed to have several utility functions for the
                benchmarks. Each function, while context-specific, is designed to
                be used in multiple scripts.

Output: Several utility functions for the benchmarks

"""
# -----------------------Package Import & Defined Arguements-------------------#
import libsbml
import importlib.util
import numpy as np
import pandas as pd
import os
import sys
import yaml


class Utils:
    """A class for storing helper functions for the benchmarks"""

    @staticmethod
    def _results_size_checker(species_data, time_trajectories):
        """Check the size of the results dictionary
        input:
            species_data: np.array - the species data
            time_trajectories: np.array - the time trajectories

        output:
            returns the species and time trajectories, if larger than 2.4gb;
            trims the results dictionary to every 1/4th of the data
        """

        size = sys.getsizeof(species_data)

        threshold_bytes = 2.4 * 1024**3

        while size > threshold_bytes:
            # cut every ith element until the size is less than the threshold
            species_data = species_data[::2]
            time_trajectories = time_trajectories[::2]

            size = sys.getsizeof(species_data)
        print(f"Size of results dictionary: {size} bytes")
        return species_data, time_trajectories

    @staticmethod
    def tasks_this_round(size, total_jobs, round_number):
        """Calculate the number of tasks for the current round
        input:
            size: int - the total number of processes assigned
            total_jobs: int - the total number of tasks

        output:
            returns the number of tasks for the current round
        """
        number_of_rounds = Utils.number_of_rounds(total_jobs, size)

        tasks_per_round = size
        remainder = total_jobs % size

        # This accounts for pythonic indexing starting at 0
        round_number += 1

        if round_number < number_of_rounds:
            tasks_this_round = tasks_per_round
        elif round_number == number_of_rounds and remainder != 0:
            tasks_this_round = remainder
        elif round_number == number_of_rounds and remainder == 0:
            tasks_this_round = tasks_per_round
        else:
            # provide an error and message exit
            raise ValueError("Round number exceeds the number of rounds")

        return tasks_this_round

    @staticmethod
    def condition_cell_id(rank_task, conditions_df, measurement_df):
        """
        Extract the condition for the task from the filtered_conditions
        output:
            returns the condition for the task
        """
        filtered_conditions = Utils._filter_conditions(conditions_df, measurement_df)

        cell = rank_task.split("+")[1]

        condition_id = rank_task.split("+")[0]

        condition = [
            condition
            for condition in filtered_conditions
            if condition["conditionId"] == condition_id
        ][0]

        return condition, cell, condition_id

    @staticmethod
    def results_dictionary(conditions_df, measurement_df):
        """Create an empty dictionary for storing results
        input:
            filtered_conditions: pd.DataFrame - filtered conditions dataframe

        output:
            returns the empty results dictionary, ready to be filled
        """

        filtered_conditions = Utils._filter_conditions(conditions_df, measurement_df)

        results = {}

        for condition in filtered_conditions:

            condition_id = condition["conditionId"]
            num_cells = condition["cell_number"] if "cell_number" in condition else 1

            for cell in range(num_cells):
                if "datasetId" in measurement_df.columns:
                    identifier = measurement_df["datasetId"]\
                        [measurement_df["simulationConditionId"] == condition_id].values[0]
                else:
                    identifier = Utils.identifier_generator()

                results[identifier] = {
                    "conditionId": condition_id,
                    "cell": cell
                }

        return results

    @staticmethod
    def number_of_rounds(total_jobs, size):
        """Calculate the number of rounds
        input:
            total_jobs: int - the total number of tasks
            size: int - the total number of processes assigned

        output:_number_of_rounds
            returns the number of rounds
        """
        rounds_to_complete = total_jobs // size

        remainder = total_jobs % size

        if remainder > 0:

            rounds_to_complete += 1

        return rounds_to_complete

    @staticmethod
    def total_tasks(conditions_df, measurement_df):
        """Calculate the total number of tasks
        input:
            conditions_df: pd.DataFrame - conditions dataframe

        output:
            returns the total number of tasks
        """

        filtered_conditions = Utils._filter_conditions(conditions_df, measurement_df)

        list_of_jobs = []

        for condition in filtered_conditions:
            if "cell_number" not in condition:
                condition_cell = f"{condition['conditionId']}+0"
                list_of_jobs.append(condition_cell)
            else:
                for cell in range(condition["cell_number"]):
                    condition_cell = f"{condition['conditionId']}+{cell}"
                    list_of_jobs.append(condition_cell)

        return list_of_jobs

    @staticmethod
    def _filter_conditions(conditions_df, measurement_df):
        """Filter the conditions dataframe to only include unique conditions
        input:
            conditions_df: pd.DataFrame - conditions dataframe

        output:
            returns the unique conditions dataframe
        """
        filtered_conditions = [
            condition
            for index, condition in conditions_df.iterrows()
            if "preequilibrationConditionId" not in measurement_df.columns
            or condition["conditionId"]
            not in measurement_df["preequilibrationConditionId"].values
        ]

        return filtered_conditions

    @staticmethod
    def assign_tasks(rank, total_jobs, size):
        """Assign tasks to ranks based on the number of jobs and the number of
            ranks
        input:
            rank: int - the rank of the current process
            total_jobs: int - the total number of tasks
            size: int - the total number of processes assigned
        output:
            returns the start and end cell for each rank
        """
        jobs_per_rank = total_jobs // int(size)
        remainder = total_jobs % int(size)

        if rank < remainder:
            rank_i_jobs = jobs_per_rank + 1
            start_cell = rank * rank_i_jobs
        else:
            rank_i_jobs = jobs_per_rank
            start_cell = rank * jobs_per_rank + remainder

        return start_cell, start_cell + rank_i_jobs

    @staticmethod  # Not even sure if this one works
    def set_compartmental_volume(
        model: libsbml.Model, compartment: str, compartment_volume: int
    ):
        """This function sets the volume of a compartment within the SBML model.
        input:
            model: libsbml.Model - the SBML model
            compartment: str - the compartment to set
            compartment_volume: int - the volume to set the compartment to
        output:
            model: libsbml.Model - the updated SBML model
        """
        # assign the compartment volume
        model.setCompartmentVolumeById(compartment, compartment_volume)

        return model

    @staticmethod
    def set_parameter_value(
        model: libsbml.Model, parameter: str, parameter_value: int
    ):
        """This function sets the value of a parameter within the SBML model.
        input:
            model: libsbml.Model - the SBML model
            parameter: str - the parameter to set
            parameter_value: int - the value to set the parameter to
        output:
            model: libsbml.Model - the updated SBML model
        """
        try:  # assign the parameter value
            model.setParameterById(parameter, parameter_value)
        except RuntimeError:
            model.setFixedParameterById(parameter, parameter_value)

        return model

    # Set this to static method to avoid the need to pass self
    @staticmethod
    def set_species_value(model: libsbml.Model, species: str, species_value: int):
        """Thiss function sets the initial value of a species or list of species
        within the sbml model.
        input:
            model: libsbml.Model - the SBML model
            species_value: int - the value to set the species to

            output:
                model: libsbml.Model - the updated SBML model"""

        # Get the list of species
        species_ids = list(model.getStateIds())

        # Get the initial values
        species_initializations = np.array(model.getInitialStates())

        # Error handling so that hard-coded species in SPARCED don't break the code
        # If an alternate model is ran through this process. 
        try:
            # Set the initial values
            index = species_ids.index(species)

            species_initializations[index] = species_value

            model.setInitialStates(species_initializations)

        except ValueError:
            pass

        return model

    @staticmethod
    def _assign_sbml_path(model_path: str):

        # Construct paths to SBML files in the specified directory
        sbml_files = [
            os.path.join(model_path, filename)
            for filename in os.listdir(model_path)
            if filename.endswith(".xml")
        ][0]

        # return the sbml file path
        return sbml_files

    @staticmethod
    def _import_module_from_path(module_path: str):
        # Add the directory containing the module to sys.path
        module_dir = os.path.dirname(module_path)
        sys.path.append(module_dir)

        # Extract the module name from the path
        module_name = os.path.splitext(os.path.basename(module_path))[0]

        # Import the module dynamically
        spec = importlib.util.spec_from_file_location(module_name, module_path)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)

        # Remove the directory from sys.path to avoid affecting other imports
        sys.path.remove(module_dir)

        return module

    @staticmethod
    def add_amici_path(model_path: str):
        """This function finds the AMICI model from the path
        input:
            model_path: str - the path to the model
        output:
            amici_module_path: str - the path to the AMICI module
        """
        # Get the directory contents
        model_path = os.path.dirname(model_path)
        try:
            directory_contents = os.listdir(model_path)
        except FileNotFoundError as e:
            raise ValueError(f"Model path '{model_path}' does not exist.") from e

        # Ignore any potential directories labeled 'results'
        directory_contents = [d for d in directory_contents if d != "results"]

        # Find the AMICI module via its setup.py file
        amici_module_path = None
        for directory in directory_contents:
            dir_path = os.path.join(model_path, directory)
            if os.path.isdir(dir_path):
                setup_path = os.path.join(dir_path, "setup.py")
                if os.path.isfile(setup_path):
                    try:
                        with open(setup_path, encoding="utf-8", mode="r") as setup_file:
                            first_line = setup_file.readline().strip()
                            if "AMICI model package setup" in first_line:
                                amici_module_path = dir_path
                                sys.path.append(amici_module_path)
                                break
                    except IOError as e:
                        print(f"Error reading setup file: {e}")

        if amici_module_path is None:
            raise ValueError("No valid AMICI module found in the provided path.")

        return amici_module_path

    @staticmethod
    def swig_interface_path(model_path: str):
        """This function finds the SWIG python interface from the path
        input:
            model_path: str - the path to the model
        output:
            swig_interface_path: str - the path to the SWIG python interface
        """
        amici_module_path = Utils.add_amici_path(model_path)

        # Get the directory contents
        try:
            amici_contents = os.listdir(amici_module_path)
        except FileNotFoundError as e:
            raise ValueError(f"AMICI module path '{amici_module_path}' does not exist.") from e

        directories = [
            item
            for item in amici_contents
            if os.path.isdir(os.path.join(amici_module_path, item))
        ]

        # Iterate over the directories to find the SWIG interface file
        swig_interface_path = None
        for directory in directories:
            directory_path = os.path.join(amici_module_path, directory)
            try:
                directory_contents = os.listdir(directory_path)
            except FileNotFoundError:
                continue

            for item in directory_contents:
                if item == directory + ".py":
                    swig_interface_path = os.path.join(directory_path, item)
                    break

            if swig_interface_path:
                break

        if swig_interface_path is None:
            raise ValueError(
                "No SWIG interface file found in the AMICI module directories."
            )

        return swig_interface_path

    @staticmethod
    def find_config_file(model_path: str, filename: str = "config.yaml") -> str:
        """
        Search for a specific configuration file (default: 'config.yaml') 
        starting from the directory of the given SBML file.

        Parameters:
            model_path (str): The path to an SBML file or directory.
            filename (str): The name of the file to search for.

        Returns:
            str: The full path to the configuration file if found.

        Raises:
            FileNotFoundError: If the configuration file cannot be found.
        """
        # Ensure we start the search from the directory containing the SBML file
        base_dir = os.path.dirname(model_path)

        # Traverse the directory to locate the config file
        for root, _, files in os.walk(base_dir):
            if filename in files:
                return os.path.join(root, filename)

        # Raise an error if the file is not found
        raise FileNotFoundError(f"{filename} not found in {base_dir} or its subdirectories.")

    @staticmethod
    def extract_simulation_files(model_path: str):
        """
        Extracts the simulation files from the directory of the SBML model.

        Parameters:
            model_path (str): The path to the SBML file.

        Returns:
            tuple: A tuple containing the gene regulation and omics data as DataFrames.
        """

        # Find the config file flexibly
        try:
            config_path = Utils.find_config_file(model_path, "config.yaml")
            print(f"Config file located: {config_path}")
        except FileNotFoundError as e:
            base_dir = os.path.dirname(model_path)
            raise ValueError(f"No config.yaml file found starting from {base_dir}") from e

        # Load the config file
        with open(config_path, encoding="utf-8", mode="r") as f:
            config = yaml.safe_load(f)

        try:
                    
            # Safely retrieve directory and files with default fallbacks
            data_files_dir = os.path.join(
                os.path.dirname(config_path),
                config["location"]
            )

            sim_file_dir = os.path.join(
                data_files_dir, 
                config["simulation"]["directory"]
            )

            # Load gene regulation file
            genereg_file = os.path.join(sim_file_dir, config["simulation"]["files"]["genes_regulation"])

            GeneReg = pd.read_csv(genereg_file, header=0, index_col=0, sep="\t")

            # Load omics data file
            omics_file = os.path.join(sim_file_dir, config["simulation"]["files"]["omics"])

            OmicsData = pd.read_csv(omics_file, header=0, index_col=0, sep="\t")

        except KeyError as e:
            raise KeyError("Missing key in config.yaml:") from e
        except FileNotFoundError as e:
            raise FileNotFoundError("Required simulation file missing:") from e
        except pd.errors.EmptyDataError as e:
            raise ValueError("File is empty or improperly formatted:") from e

        return GeneReg, OmicsData


    @staticmethod
    def set_transcription_values(
        omics_data: pd.DataFrame, gene: str, value: int
    ) -> None:
        """This function sets the value of a parameter within the SBML model.
        input:
            model_path:  model_path: str - the path to the model
            gene: str - the gene to knockout
            value: int - the value to set the gene to
        output:
            model: libsbml.Model - the updated SBML model
        """

        # Update values
        omics_data.loc[gene, "kTCleak"] = 0.0
        omics_data.loc[gene, "kTCmaxs"] = 0.0
        omics_data.loc[gene, "kTCd"] = 0.0

        omics_data.loc[gene, "Exp RNA"] = value

        return omics_data

    @staticmethod
    def identifier_generator():
        """This function generates a unique identifier for the iterative
            of each simulation process.
        output:
            returns the unique identifier
        """
        import uuid

        identifier = str(uuid.uuid4())

        del uuid

        return identifier
    
