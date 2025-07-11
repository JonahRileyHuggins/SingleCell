#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
script name: job_organization.py
Created on Fri Jan. 10th 12:00:00 2024
Author: Jonah R. Huggins

Description: Curates and handles the organization of tasks for the MPI processes

Output: MPI tasks for each rank, MPI task assignment, and MPI results aggregation
        results storage.

"""
# -----------------------Package Import & Defined Arguements-------------------#
import sys
import numpy
import importlib
import pandas as pd
import mpi4py.MPI as MPI
from src.utils.petab_file_loader import PEtabFileLoader
from src.validation.simulation.utils import Utils


def mpi_communicator() -> MPI.Comm:
    """This function initializes the MPI communicator
    Input:
        None
    Output:
        comm: MPI communicator - the MPI communicator
    """

    communicator = MPI.COMM_WORLD
    rank = communicator.Get_rank()
    size = communicator.Get_size()

    return communicator, rank, size

def broadcast_petab_files(
    rank: int, communicator: MPI.Comm, yaml_file: str
) -> pd.DataFrame:
    """
    Broadcasts the PEtab files to all ranks

    Parameters
    - rank (int): the rank of the MPI process
    - communicator (MPI.Comm): the MPI communicator
    - yaml_file (str): the path to the yaml file

    Returns
    - sbml_file (str): the path to the SBML file
    - conditions_df (pandas.DataFrame): the conditions dataframe
    - measurement_df (pandas.DataFrame): the measurement dataframe
    - observable_df (pandas.DataFrame): the observable dataframe
    - parameters_df (pandas.DataFrame): the parameters dataframe
    - visualization_df (pandas.DataFrame): the visualization\
    dataframe
    """

    if rank == 0:
        petab_files = PEtabFileLoader(yaml_file).__call__()
        petab_files_data = {
            "sbml_file": petab_files.sbml_file,
            "conditions_df": petab_files.conditions_df,
            "measurement_df": petab_files.measurement_df,
            "observable_df": petab_files.observable_df,
            "parameter_df": petab_files.parameter_df,
        }

        if "visualization_df" in petab_files.__dict__:
            petab_files_data["visualization_df"] = petab_files.visualization_df

        sbml_file = petab_files_data["sbml_file"]
        conditions_df = petab_files_data["conditions_df"]
        measurement_df = petab_files_data["measurement_df"]
        observable_df = petab_files_data["observable_df"]
        parameters_df = petab_files_data["parameter_df"]

        visualization_df = (
            petab_files_data["visualization_df"]
            if "visualization_df" in petab_files_data
            else None
        )

    else:
        petab_files_data = None
        # Broadcasting from root avoids Race Conditions
    petab_files_data = communicator.bcast(petab_files_data, root=0)

    if rank != 0:
        sbml_file = petab_files_data["sbml_file"]
        conditions_df = petab_files_data["conditions_df"]
        measurement_df = petab_files_data["measurement_df"]
        observable_df = petab_files_data["observable_df"]
        parameters_df = petab_files_data["parameter_df"]

        visualization_df = (
            petab_files_data["visualization_df"]
            if "visualization_df" in petab_files_data
            else None
        )

    return (
        sbml_file,
        conditions_df,
        measurement_df,
        observable_df,
        parameters_df,
        visualization_df,
    )

def broadcast_simulation_files(sbml_file: str, communicator: MPI.Comm, rank: int) -> tuple:
    """
    Broadcasts the simulation files to all ranks.

    Parameters:
        sbml_file (str): The path to the SBML file.
        communicator (MPI.Comm): The MPI communicator.
        rank (int): The rank of the MPI process.

    Returns:
        tuple: The gene regulation and omics data as pandas DataFrames.
    """
    simulation_files = None

    try:
        if rank == 0:
            # Extract simulation files on rank 0
            genereg, omicsdata = Utils.extract_simulation_files(sbml_file)

            # Package data for broadcasting
            simulation_files = {
                "genereg": genereg,
                "omicsdata": omicsdata
            }

        # Broadcast simulation files from rank 0 to all ranks
        simulation_files = communicator.bcast(simulation_files, root=0)

        # Extract data from the broadcasted dictionary
        genereg = simulation_files["genereg"]
        omicsdata = simulation_files["omicsdata"]

    except Exception as e:
        # Ensure all ranks print the error and continue as AMICI
        print(f"Rank {rank}: {e}")
        print("No gene regulation or omics data files found. Continuing as Generic AMICI model.")

        # Set default values for all ranks
        genereg = None
        omicsdata = None

        # Ensure all ranks participate in the broadcast
        simulation_files = communicator.bcast({"genereg": genereg, "omicsdata": omicsdata}, root=0)

    return genereg, omicsdata

def task_organization(
    rank: int,
    size: int,
    communicator: MPI.Comm,
    conditions_df: pd.DataFrame,
    measurement_df: pd.DataFrame,
) -> dict:
    """This function assigns tasks to each rank
    Input:
        rank: int - the rank of the MPI process
        size: int - the number of MPI processes
        n_tasks: int - the number of tasks to be distributed
    Output:
        task_list: list - the list of tasks assigned to the rank
    """

    list_of_jobs = Utils.total_tasks(conditions_df, measurement_df)

    total_jobs = len(list_of_jobs)

    rank_jobs_directory = {}

    for i in range(size):

        start_cell, end_cell = Utils.assign_tasks(i, total_jobs, size)

        rank_jobs_directory[i] = list_of_jobs[start_cell:end_cell]

    # Assign each rank it's task for the round
    rounds_to_complete = Utils.number_of_rounds(total_jobs, size)

    return rounds_to_complete, rank_jobs_directory

def task_assignment(
    rank: int,
    size: int,
    communicator: MPI.Comm,
    rank_jobs_directory: dict,
    round_i: int,
    conditions_df: pd.DataFrame,
    measurement_df: pd.DataFrame,
) -> dict:

    _, rank_jobs_directory = task_organization(
        rank, size, communicator, conditions_df, measurement_df
    )
    if rank == 0:
        # Assign each rank it's task for the round
        for i in range(size):

            rank_jobs = rank_jobs_directory[i]

            if round_i < len(rank_jobs):
                rank_job_for_round = rank_jobs[round_i]

            else:
                rank_job_for_round = None

            communicator.send(rank_job_for_round, dest=i, tag=round_i)

    # Receive the task for the round
    rank_task = communicator.recv(source=0, tag=round_i)

    communicator.Barrier()

    return rank_task

def package_results(
    results: pd.DataFrame,
    condition_id: str,
    cell: str,
) -> None:
    """
    This function gathers all of the results from the simulation into a single
    dictionary to simplify the process of sending the results to the root rank

    Input:
        results: pd.DataFrame - the results dataframe
        condition_id: str - the condition identifier
        cell: str - the cell identifier

    Output:
        rank_results: dict - the results dictionary for the rank
    """

    # make a dict entry in rank_results for every column in results
    rank_results = {
        "conditionId": condition_id,
        "cell": int(cell),
        "results": results,
    }

    return rank_results

def aggregate_other_rank_results(
    size: int, communicator, results_dict: dict, round_i: int, total_jobs: int
) -> dict:
    """This function aggregates the results from the ranks and
    stores them in the final simulation results dictionary

    Input:
        size: int - the number of MPI processes
        communicator: MPI communicator - the MPI communicator
        results: dict - the results dictionary
        results: dict - the results dictionary
        round_i: int - the current round
        total_jobs: int - the total number of jobs

        Output:
            results_dict: dict - the results dictionary"""

    # Determine the number of tasks to be completed this round, subtract 1
    # to account for the root rank saving results prior.
    tasks_this_round = Utils.tasks_this_round(size, total_jobs, round_i) - 1

    completed_tasks = 0

    while completed_tasks < tasks_this_round:

        results = communicator.recv(source=MPI.ANY_SOURCE, tag=MPI.ANY_TAG)

        results_dict = store_results(results, results_dict)

        completed_tasks += 1

        if completed_tasks == tasks_this_round:
            break

    return results_dict

def store_results(individual_parcel: dict, results_dict: dict) -> dict:
    """This function stores the results in the results dictionary

    Input:
        individual_parcel: dict - individual simulation results
        results_dict: dict - the results dictionary

    Output:
        results_dict: dict - the results dictionary"""

    condition_id = individual_parcel["conditionId"]
    cell = individual_parcel["cell"]

    # Find the matching entry in results_dict using `.get()`
    for _, value in results_dict.items():

        if value.get("conditionId") == condition_id and value.get("cell") == cell:

            for column in individual_parcel["results"].columns:

                value[column] = individual_parcel["results"][column]
                
            break  # Stop searching once we find the match

    return results_dict
