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
import pickle
import shutil
from types import SimpleNamespace
from collections import defaultdict, deque

import pandas as pd
import mpi4py.MPI as MPI

import shared_utils.file_loader as l


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

def broadcast_files(
    rank: int, 
    communicator: MPI.Comm, 
    yaml_file: str
) -> SimpleNamespace:
    """
    Broadcasts the PEtab files to all ranks

    Parameters
    - rank (int): the rank of the MPI process
    - communicator (MPI.Comm): the MPI communicator
    - yaml_file (str): the path to the yaml file

    Returns
    - loader (SimpleNameSpace): object containing loaded files from Petab configuration file.
    """

    if rank == 0:
        loader = l.FileLoader(yaml_file)

        loader._petab_files()

    else:
        loader = None

    loader = communicator.bcast(loader, root=0)

    return loader

def task_organization(
    size: int,
    measurement_df: pd.DataFrame, 
    cell_count: int
) -> dict:
    """Assigns tasks to each rank
    Input:
        rank: int - the rank of the MPI process
        size: int - the number of MPI processes
        details (dict): petab yaml file containing paths to PEtab files.
    Output:
        task_list: list - the list of tasks assigned to the rank
    """
    list_of_jobs = total_tasks(measurement_df, cell_count, size)

    rank_jobs_directory = {}

    for i in range(size):
        
        rank_ids = assign_tasks(i, len(list_of_jobs), size)

        jobs_list = [list_of_jobs[job] for job in rank_ids]

        rank_jobs_directory[i] = jobs_list

    # Assign each rank it's task for the round
    rounds_to_complete = -(-len(list_of_jobs) // size)

    return rounds_to_complete, rank_jobs_directory

def topo_sort_conditions(measurements_df: pd.DataFrame) -> list:
    """
    Given a DataFrame with columns
      - 'simulationConditionId'
      - 'preequilibrationConditionId'
    return a list of unique simulationConditionIds in dependency order
    (i.e. all pre‐equilibration conditions come before their dependents).
    """
    # 1) Collect all nodes
    nodes = set(measurements_df['simulationConditionId'].dropna()) \
          | set(measurements_df['preequilibrationConditionId'].dropna())
    
    # 2) Build adjacency list and in‐degree map
    succs = defaultdict(list)   # prerequisite → [dependents…]
    indegree = {n: 0 for n in nodes}
    
    for _, row in measurements_df.dropna(subset=['preequilibrationConditionId']).iterrows():
        pre = row['preequilibrationConditionId']
        sim = row['simulationConditionId']
        succs[pre].append(sim)
        indegree[sim] += 1
    
    # 3) Kahn’s algorithm for topological sort
    queue = deque(n for n, d in indegree.items() if d == 0)
    ordered = []
    
    while queue:
        n = queue.popleft()
        ordered.append(n)
        for m in succs[n]:
            indegree[m] -= 1
            if indegree[m] == 0:
                queue.append(m)
    
    if len(ordered) != len(nodes):
        raise RuntimeError("Circular dependency detected among conditions!")
    
    return ordered

def __delay_post_conditions(
        measurement_df: pd.DataFrame, 
        task_list: list, 
        cell_count: int, 
        size: int
    ) -> list:
    
    """Inserts into topologically sorted task list (with cells) `None` values 
    to delay conditions with pre-conditional dependency simulations"""
    
    pre_conds = measurement_df['preequilibrationConditionId'].drop_duplicates().dropna().to_list()
    # Since this is only called after topological sorting via Khan's alg., all 0-order conditions 
    # are first; the task_list is already ordered!

    for idx, job in enumerate(task_list):

        if job == None:
            continue

        cond_id = job.split("+")[0]

        if cond_id in pre_conds:

            pause_ranks = max((size - cell_count), 0)

            while pause_ranks:

                task_list.insert(idx+1, None)

                pause_ranks -= 1

    return task_list


def total_tasks(measurements_df: pd.DataFrame, cell_count: int, size: int) -> list:
    """Calculate the total number of tasks from the measurement dataframe"""

    if 'preequilibrationConditionId' in measurements_df.columns:
        # Reorder conditions with 0-order dependencies first:
        ordered_conditions = topo_sort_conditions(measurements_df)

    else:
        ordered_conditions = measurements_df['simulationConditionId'].unique().tolist()

    list_of_jobs = []
    for cond in ordered_conditions:
        for cell in range(1, cell_count + 1):
            list_of_jobs.append(f"{cond}+{cell}")

    if 'preequilibrationConditionId' in measurements_df.columns: 
        # Add delays for dependent conditions & cells; requires cell number in job ID
        list_of_jobs = __delay_post_conditions(
            measurements_df,
            list_of_jobs, 
            cell_count, 
            size
            )

    return list_of_jobs

def assign_tasks(rank: int, total_jobs: int, size: int) -> list:
    """
    Assign tasks to a given rank in round-robin fashion, including those that are part
    of sequential chains. Ensures layering of job dependencies, avoids
    oversubscription what the longest sequential dependency chain requires.
    """

    num_rounds = -(-total_jobs // size)  # Equivalent to math.ceil(total_jobs / size)
    
    rank_jobs = []
    
    for round_index in range(num_rounds):
        job_id = rank + round_index * size
        if job_id < total_jobs:
            rank_jobs.append(job_id)

    return rank_jobs

def task_assignment(
    rank: int,
    size: int,
    communicator: MPI.Comm,
    rank_jobs_directory: dict,
    round_i: int
) -> dict:

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

def condition_cell_id(rank_task, conditions_df):
    """
    Extract the condition for the task from the filtered_conditions
    output:
        returns the condition for the task
    """

    cell = rank_task.split("+")[1]

    condition_id = rank_task.split("+")[0]

    matches = conditions_df.loc[conditions_df["conditionId"] == condition_id]

    if matches.empty:
        raise ValueError(f"Condition ID '{condition_id}' not found in conditions_df")

    condition = matches.iloc[0]

    return condition, cell, condition_id

def package_results(
    results: pd.DataFrame,
    condition_id: str,
    cell: str,
) -> dict:
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
        communicator: MPI.Intracomm,
        results_dict: dict,
        size: int
) -> dict:
    """This function aggregates the results from the ranks and
    stores them in the final simulation results dictionary

    Input:
        communicator: MPI communicator - the MPI communicator
        results: dict - the results dictionary
        size: integer of number of ranks performing jobs

    Output:
        results_dict: dict - the results dictionary"""
    
    #  Add 1 to account for the root rank saving results prior.
    completed_tasks = 1

    task_counter =  size

    while completed_tasks < task_counter: 

        results = communicator.recv(source=MPI.ANY_SOURCE, tag=MPI.ANY_TAG)

        if results is None:
            task_counter -= 1
        
        else:
            results_dict = store_results(results, results_dict)
            completed_tasks += 1

        if completed_tasks == task_counter:
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


class ResultCache:

    def __init__(self, cach_dir: str = './.cache_') -> None:
        self.cache_dir = cach_dir

        os.makedirs(self.cache_dir, exist_ok=True)

    def _key_to_path(self, key: str) -> str:
        """Convert a dictionary key to a safe file path"""

        return os.path.join(self.cache_dir, f"{key}.pkl")

    def save(self, key: str, df: pd.DataFrame) -> None:
        """Save a single DataFrame under a key"""

        path = self._key_to_path(key)

        with open(path, 'wb') as f:
            pickle.dump(df, f)

    def load(self, key: str) -> pd.DataFrame:
        """Load a single DataFrame by key"""

        path = self._key_to_path(key)

        with open(path, 'rb') as f:
            return pickle.load(f)

    def delete_cache(self) -> None:
        """Removes cache directory after results have been saved."""
        shutil.rmtree(self.cache_dir, ignore_errors=False)