#!/bin/env python3 
"""
script name: RunSPARCED.py
Created on Thurs. 2025-04-12 20:29:00
Author: Jonah R. Huggins

Description: [**PROTOTYPE**] entrypoint for the SPARCED model simulation. Parsing and wrapper \n
function that takes model attributes, performs basic data handling, and passes \n
initial model state array to SPARCED C++ simulation engine  

Input: Simulation Settings

Output:
    Simulation Results

"""
# -----------------------Package Import & Defined Arguements-------------------#

import os 
import time
import libsbml
import numpy as np

#-------------------Class Definition-----------------------------------------#
class _SBMLHandler:
    """Miscelaneous functions for working with the SBML files."""

    def __init__(self, sbml_path: str | os.PathLike):
        """Loads an instance of an SBML, stores helper functions."""
        # Read-in the model SBML to get compartmental volumes (used to convert nM to mpc and vice versa)
        sbml_reader = libsbml.SBMLReader()
        
        self.sbml_doc = sbml_reader.readSBML(sbml_path)

        self.sbml_model = self.sbml_doc.getModel()

    def _get_initial_model_states(self) -> list[float]:
        """
        Retrieves model initial species values.
        """

        initial_sbml_states = []

        # Loop through species and get initial concentrations or amounts
        for i in range(self.sbml_model.getNumSpecies()):
            species = self.sbml_model.getSpecies(i)

            if species.isSetInitialConcentration():
                initial_value = species.getInitialConcentration()

            elif species.isSetInitialAmount():
                initial_value = species.getInitialAmount()

            else:
                initial_value = 0.0  
        
            initial_sbml_states.append(initial_value)

        return initial_sbml_states

    @staticmethod
    def _set_species_values(changing_states: list, 
                        species_names: list, 
                        target: dict) -> np.ascontiguousarray:
        """
        Iterates over list of species to be changed, assigns them to an initial state vector

        Parameters:
        - changing_states (list): states recieved that need to be changed within the model class
        - species_names (list): Model-species' names 

        Returns:
        - initial_states (np.ndarray): species-length array of values, updated with user-defined values
        """

        for index, species in enumerate(species_names):
            if species in target:
                changing_states[index] = target[species]

        return changing_states