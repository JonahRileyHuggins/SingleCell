#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
script name: viz_helpers.py
Created on Jan. 5th, 2025
Author: Jonah R. Huggins

Description: This script is designed as a helper script to visualizations
             for various datasets in the SPARCED model.
"""

#-----------------------Package Import & Defined Arguements-------------------#
import pickle
from typing import Optional, Tuple

import numpy as np
import matplotlib.pyplot as plt
from matplotlib.gridspec import GridSpec

#-----------------------Class Definitions-------------------------------------#

class Helpers:
    """
    This class is designed to enable easy access and organization of various\
     SPARCED model visualizations.
    """
    @staticmethod
    def load_data(file_path):
        """Load the pickle data from the specified file."""
        with open(file_path, 'rb') as f:
            return pickle.load(f)
    
    @staticmethod
    def process_value(value):
        """Ensure the value is numeric, extracting from a list if necessary."""
        if isinstance(value, list):
            if len(value) == 1:
                return value[0]
            raise ValueError("Expected a single-item list, but got multiple items.")
        return value
    
    @staticmethod
    def experiment_to_list(data: dict, key: str):
        """Convert the experiment data to a list."""
        return [data[condition][key] for condition in data]
    
    @staticmethod
    def extract_experimental_data(data: dict, observable_name: str):
        """Extract the experimental data from the dictionary."""

        data_dict = {}
        for simulation in data:

            data_dict[simulation] = {
                'time': data[simulation][observable_name]['time'],
                'conditionId': data[simulation]['conditionId'],
                'cell': data[simulation]['cell']
            }

            for key in data[simulation][observable_name]: 

                if 'experiment' in key:
                    data_dict[simulation][key] = data[simulation][observable_name][key]

        return data_dict


class CellDeathMetrics:
    """
    Functions for calculating SPARCED cell population death metrics
    """
    def __init__(self, data, observable_name):
        """ This is extended functionality for the observable calculator class. 
        It is designed to calculate different death point metrics for each cell in the simulation results.

        data: dictionary containing the simulation results from ObservableCalculator
        observable_name: name of the observable to be used to determine time of death
        """
        self.data = data
        self.observable_name = observable_name


    def time_to_death(self, threshold=100.0):
        """"Returns the time for each simulated cell death for each condition in the results dictionary
        
        output: dictionary containing the times to death for each cell per condition"""

        time_of_death = {}
        for entry in self.data:
            time_of_death[entry] = {}
            time_of_death[entry]['value'] = []

            # Extract time array and ensure it's 1D
            time_array = np.array(self.data[entry][self.observable_name]['time']).ravel()
            simulation_values = np.array(self.data[entry][self.observable_name]['simulation'])

            # Ensure the boolean mask matches the shape of time_array
            mask = simulation_values > threshold
            
            # Handle cases dynamically
            if time_array.size == 1:  # Single-value case
                if mask.any():  # If at least one condition exceeds the threshold
                    dead_simulation_times = time_array[0]  # Use the single time value
                else:
                    dead_simulation_times = np.nan
            else:  # Standard multi-value case
                dead_simulation = time_array[mask]  # Apply boolean mask
                dead_simulation_times = dead_simulation[0] if dead_simulation.size > 0 else np.nan

            # Store results
            time_of_death[entry]['value'].append(dead_simulation_times)
            time_of_death[entry]['conditionId'] = self.data[entry]['conditionId']
            time_of_death[entry]['cell'] = self.data[entry]['cell']

        # Final time_of_death variable contains len(data.keys()) matching entries
        # each with the corresponding conditionId and cell number, as well as the 
        # time in which they died.
        return time_of_death


    def average_time_to_death(self):
        """Returns the average time to death for each condition in the results dictionary
        
        output: dictionary containing the average time to death for each condition"""

        time_of_death = self.time_to_death()

        condition_averaged_times = {}

        for _, value in time_of_death.items():
            condition = value['conditionId']
            time = value['value']
            if condition not in condition_averaged_times:
                condition_averaged_times[condition] = []
            
            condition_averaged_times[condition].append(time)

        for condition, times in condition_averaged_times.items():
            condition_averaged_times[condition] = np.mean(times)
        
        return condition_averaged_times

    def death_ratio(self, percent:Optional[bool] = False):
        """ Returns the ratio of dead cells for each condition in the results\
              dictionary
        Parameters:
        - time_to_death (dict): dictionary containing the times to death for \
            each cell per condition from the time_to_death function

        Returns:    
        - dead_cells (dict): dictionary containing the ratio of dead cells for \
            each condition"""

        dead_cells = {}

        cells_per_condition = {}
        for _, entry_info in self.time_to_death().items():
            condition = entry_info['conditionId']
            if condition not in dead_cells:
                dead_cells[condition] = 0

            if entry_info['value'][0] is not np.nan:
                dead_cells[condition] += 1

            if condition not in cells_per_condition:
                cells_per_condition[condition] = 0
            cells_per_condition[condition] += 1

        for condition, dead_count in dead_cells.items():
            dead_cells[condition] = dead_count / cells_per_condition[condition]
            if percent:
                dead_cells[condition] = dead_cells[condition] * 100

        return dead_cells
    
    def alive_ratio(self, percent:Optional[bool] = False):
        """Returns the ratio of alive cells, should be proceeded by collect_the_dead function
        
        output: dictionary containing the ratio of alive cells for each condition"""

        death_ratio = self.death_ratio()
        if percent is True:
            alive_ratio = [(1 - x)*100 for x in death_ratio.values()]
        else:
            alive_ratio = [(1 - x) for x in death_ratio.values()]

        return alive_ratio 

    @staticmethod
    def calculate_dead_cells(dead_cells, CELLS_PER_CONDITION):
        """Calculate the percentage of dead cells at different time points."""
        dead_cells_24to72 = {}
        for simulation in dead_cells:
            condition_id = dead_cells[simulation]['conditionId']
            value = Helpers.process_value(value=dead_cells[simulation]['value'])

            # Handle NaN values
            if np.isnan(value):
                value = 100 * 3600

            # Initialize dictionary if not present
            if condition_id not in dead_cells_24to72:
                dead_cells_24to72[condition_id] = {'24': [], '48': [], '72': []}

            # Categorize based on time thresholds
            for threshold in ['24', '48', '72']:
                if value <= int(threshold) * 3600:
                    dead_cells_24to72[condition_id][threshold].append(1)

        # Calculate percentages
        for condition_id, _ in dead_cells_24to72.items():
            for key in dead_cells_24to72[condition_id]:
                dead_cells_24to72[condition_id][key] = len(dead_cells_24to72[condition_id][key]) / CELLS_PER_CONDITION * 100

        return dead_cells_24to72

    @staticmethod
    def match_calculate_dead_cells_struct(experimental_data: dict):
        """
        Takes the experimental data and matches the dictionary format of the calculated dead cells.

        Parameters:
        - experimental_data (dict): Dictionary containing the experimental data.

        Returns:
        - dict: Dictionary with the same structure as the calculated dead cells.
        """

        matched_dict = {}
        processed_conditions = set()

        for exp_info in experimental_data.values():
            condition_id = exp_info['conditionId']

            # Skip conditions that have already been processed
            if condition_id in processed_conditions:
                continue

            # Initialize the nested structure for the new condition
            matched_dict[condition_id] = {'24': np.nan, '48': np.nan, '72': np.nan}

            for key, value in exp_info.items():
                if key == 'experiment':
                    for idx, time_point in enumerate(['24', '48', '72']):
                        # Only assign if the index is within the length of the value array
                        if idx < len(value):
                            matched_dict[condition_id][time_point] = (
                                value[idx] if value[idx] is not None else np.nan
                            )

            # Mark the condition as processed
            processed_conditions.add(condition_id)

        return matched_dict


class CellPopMetrics:
    """
    This class is designed to enable easy access and organization of cell population metrics
    for the SPARCED model. It takes in a dictionary of loaded pickle results and creates a registry.
    """
    def __init__(self, data: dict):
        """
        Initializes the CellPopMetrics instance and creates a registry
        of condition-matched cells.

        Parameters:
        - data (dict): Dictionary of loaded pickle results.
        """
        self.registry = {}
        
        for simulation in data:
            # Gather the condition identifier and cell #
            condition_id = data[simulation]['conditionId']

            # Add the simulation to the appropriate list in the registry
            self.registry.setdefault(condition_id, []).append(simulation)

        self.data = data

    def get_registry(self) -> dict:
        """
        Returns the registry of condition-matched cells.

        Returns:
        - dict: The registry with conditions as keys and lists of simulations as values.
        """
        return self.registry
    
    def cells_above_threshold(self, observable: str, threshold: int, experimental_data: Optional[str] = None) -> dict:
        """ Returns the ratio of cells above a certain threshold for each condition in the results dictionary """
        threshold_bins = {}

        conds_registry = self.get_registry()

        cycling_val = 0

        for simulation in self.data:
            condition_id = self.data[simulation]['conditionId']

            if condition_id not in threshold_bins:
                threshold_bins[condition_id] = {}
                threshold_bins[condition_id]['sim'] = []

            if any(value > threshold for value in self.data[simulation][observable]['simulation']):
                threshold_bins[condition_id]['sim'].append(1)
                cycling_val += 1
                
            if experimental_data:
                threshold_bins[condition_id]['exp'] = self.data[simulation][observable]['experiment']

        for condition_id, _ in threshold_bins.items():
            threshold_bins[condition_id]['sim'] = len(threshold_bins[condition_id]['sim']) / len(conds_registry[condition_id]) * 100

        return threshold_bins


class LeftRightSplit:
    """
    functions for splitting datasets into left and right visuals with the gridspec library.
    """
    def __init__(self, nrows, ncols, figsize: Optional[tuple] = None, width_ratios:Optional[list] = None):
        # Create figure and GridSpec layout
        fig = plt.figure(figsize=figsize)
        gs = GridSpec(nrows, ncols, figure=fig, width_ratios=width_ratios)

        # Plotting
        axes_left = [fig.add_subplot(gs[i, 0]) for i in range(nrows)]
        axes_right = [fig.add_subplot(gs[i, 1]) for i in range(nrows)]

        self.fig = fig
        self.axes_left = axes_left
        self.axes_right = axes_right
        self.gs = gs

    def plot_left(self, data, conds_registry, observable, dependent_var, independent_var,
                  colors: Optional[list] = None,
                  yaxis_limits: Optional[Tuple[int, int]] = (0, 400),
                  xaxis_limits: Optional[Tuple[int, int]] = (0, 72)):
        """Create the left plots."""

        CONVERT_TO_HOURS = 3600

        for i, condition in enumerate(conds_registry):

            color = colors[i] if colors else None

            for simulation in data:
                if simulation in conds_registry[condition]:
                    self.axes_left[i].plot(
                        data[simulation][observable][dependent_var] / CONVERT_TO_HOURS,
                        data[simulation][observable][independent_var], 
                        linewidth=4, color=color
                    )
            self.axes_left[i].set_ylim(yaxis_limits[0], yaxis_limits[-1])
            self.axes_left[i].set_xlim(xaxis_limits[0], xaxis_limits[-1])
            self.axes_left[i].set_xticklabels(self.axes_left[i].get_xticks(), fontsize=16, weight='bold')
            self.axes_left[i].set_yticklabels(self.axes_left[i].get_yticks(), fontsize=16, weight='bold')

    def plot_left_bar(self, data, error_bars=True, x_labels=None, y_range=None, colors=None, CELLS_PER_CONDITION=1):
        """
        Generalized function to plot bar charts on given axes.

        Parameters:
        - axes (list): List of matplotlib axes to plot on.
        - data (dict): Dictionary where keys are condition IDs and values are data for each bar plot.
        - x_labels (list, optional): List of labels for the x-axis. If None, use keys from data.
        - y_range (tuple, optional): Tuple specifying the (min, max) range for the y-axis. Default is None.
        - colors (list, optional): List of colors for the bars. Default is None.
        """

        for i, condition_id in enumerate(data):
            # Extract values ensuring scalars are used
            values = [data[condition_id][key] if not isinstance(data[condition_id][key], np.ndarray)
                    else data[condition_id][key].item()
                    for key in data[condition_id]]

            std_err = None
            if error_bars:

                std_err = np.array([
                    np.sqrt(val * (100 - val) / CELLS_PER_CONDITION) 
                    for val in values
                ])

            color = colors[i] if colors else None

            self.axes_left[i].bar(data[condition_id].keys(), values, yerr=std_err, color=color)

            # Set y-axis limits if specified
            if y_range:
                self.axes_left[i].set_ylim(y_range)

            # Styling the y-tick labels
            self.axes_left[i].set_yticklabels(self.axes_left[i].get_yticks(), fontsize=16, weight='bold')

            # Custom x-tick labels if provided, else show index labels
            if x_labels and i == len(self.axes_left) - 1:  # Only set x-tick labels on the last subplot
                self.axes_left[i].set_xticklabels(x_labels, fontsize=16, weight='bold')
            elif i < len(self.axes_left) - 1:
                self.axes_left[i].set_xticks([]) #Hide x-ticks for non-bottom plots



    def plot_right(self, data, error_bars=True, x_labels=None, y_range=None, colors=None, CELLS_PER_CONDITION=1):
        """
        Generalized function to plot bar charts on given axes.

        Parameters:
        - axes (list): List of matplotlib axes to plot on.
        - data (dict): Dictionary where keys are condition IDs and values are data for each bar plot.
        - x_labels (list, optional): List of labels for the x-axis. If None, use keys from data.
        - y_range (tuple, optional): Tuple specifying the (min, max) range for the y-axis. Default is None.
        - colors (list, optional): List of colors for the bars. Default is None.
        """
        for i, condition_id in enumerate(data):
            # Extract values ensuring scalars are used
            values = [data[condition_id][key] if not isinstance(data[condition_id][key], np.ndarray)
                    else data[condition_id][key].item()
                    for key in data[condition_id]]

            std_err = None
            if error_bars:  # Plot error bars if provided
                std_err = np.array([
                    np.sqrt(val * (100 - val) / CELLS_PER_CONDITION) 
                    for val in values
                ])

            if colors and len(colors) == len(data):
                color = colors[i]

            elif colors and len(colors) != len(data):
                color = colors
            
            else: 
                color = None

            self.axes_right[i].bar(data[condition_id].keys(), values, yerr=std_err, color=color)

            # Set y-axis limits if specified
            if y_range:
                self.axes_right[i].set_ylim(y_range)
            
            # Styling the y-tick labels
            self.axes_right[i].set_yticklabels(self.axes_right[i].get_yticks(), fontsize=16, weight='bold')
            
            # Custom x-tick labels if provided, else show index labels
            if x_labels and i == len(self.axes_right) - 1:  # Only set x-tick labels on the last subplot
                self.axes_right[i].set_xticklabels(x_labels, fontsize=16, weight='bold')
            elif i < len(self.axes_right) - 1:
                self.axes_right[i].set_xticks([])  # Hide x-ticks for non-bottom plots

    def add_text(self, text, x, y, fontsize=16, weight='bold'):
        """
        Add text to the specified axes.

        Parameters:
        - text (str): Text to add to the axes.
        - x (float): X-coordinate for the text.
        - y (float): Y-coordinate for the text.
        - axes (matplotlib.axes.Axes): Axes to add the text to.
        - fontsize (int, optional): Font size for the text. Default is 16.
        - weight (str, optional): Font weight for the text. Default is 'bold'.

        Returns:
        - matplotlib.text.Text: Text object added to the axes.
        """
        self.fig.text(x, y, text, fontsize=fontsize, weight=weight)

    def turn_off_axes(self):
        """Turn off the axes for the figure."""
        for ax in self.axes_left + self.axes_right:
            ax.axis('off')

    def disable_axes_labels(self, axes=None, x_axis=False, y_axis=False):
        """
        Disables a particular axis label on the specified axes.

        Parameters:
        - axes (list, optional): List of axes to disable the labels on. Default is None.
        - x_axis (bool, optional): Whether to disable the x-axis labels. Default is False.
        - y_axis (bool, optional): Whether to disable the y-axis labels. Default is False.

        Returns:
        """
        if axes is None or axes == 'all':
            axes = self.axes_left + self.axes_right

        elif axes is 'left':
            axes = self.axes_left

        elif axes is 'right':
            axes = self.axes_right

        for ax in axes:
            if x_axis:
                ax.set_xticklabels([])
            if y_axis:
                ax.set_yticklabels([])

    def show(self):
        """Show the figure."""
        plt.show()

    def save_plot(self, file_name, dpi=300):
        """Save the plot to a file."""
        self.fig.savefig(file_name, dpi=dpi)
