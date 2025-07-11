#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
script name: sparced_simulation.py
Created on Thurs. 04/23/2024 9:00:00
Author: Jonah R. Huggins

Description: This file conducts user defined, condition-specific simulations \
using SPARCED and returns the results as nested NumPy arrays.

Output:
- xoutS_all (np.ndarray): The species concentrations at each timepoint
- tout_all (np.ndarray): The timepoints at which the species concentrations \
    were recorded
- xoutG_all (np.ndarray): The gene expression values at each timepoint

"""

# -----------------------Package Import & Defined Arguements-------------------#
import sys
import math
import libsbml
import importlib
import numpy as np
import pandas as pd

from utils import Utils as utils
from shared_utils.combine_results import combine_results

sys.path.append("../../../build/")
from pySingleCell import SingleCell

class Simulator:
    def __init__(
        self,
        yaml_file: str,
        conditions_df: pd.DataFrame,
        measurement_df: pd.DataFrame,
        parameters_df: pd.DataFrame
    ):
        """
        This class is designed to simulate the experimental replicate model.
        input:
            yaml_file: str - path to the YAML file
            model: str - path to the SBML model
            conditions_df: pd.DataFrame - the conditions dataframe
            measurement_df: pd.DataFrame - the measurement dataframe
            parameters_df: pd.DataFrame - the parameters dataframe
            sbml_file: str - path to the SBML file
        """

        self.yaml_file = yaml_file
        self.conditions_df = self._remove_condition_name(conditions_df)
        self.perturbants = self._extract_perturbations(self.conditions_df)
        self.measurement_df = measurement_df
        self.parameters_df = parameters_df

        # Load the SPARCED model
        self.model = self.load_sparced_model()

    def run(self, condition: pd.Series) -> pd.DataFrame:
        """This function runs the simulation for a single condition.
        Parameters:
            self: Simulation - the Simulation class
            condition: pd.Series - the condition to simulate

        Returns:
            result: pd.DataFrame - the simulation results
        """

        # Look for heterogenize parameters in the condition
        self.model = self.heterogenize(condition)

        # Look for preequilibration parameters in the condition
        self.model = self.preequilibrate(condition)

        # Set the perturbations for the simulation
        perturbant_handler = Perturbations(self)
        
        perturbant_handler.set_perturbations(condition)

        # Set the timepoints for the simulation
        simulation_timeframe = self.measurement_df["time"][
            self.measurement_df["simulationConditionId"].isin(condition)
        ].max()

        perturbant_handler.model.setTimepoints(np.linspace(0, 30))


        # Run the simulation
        xoutS_all, xoutG_all, tout_all = RunSPARCED(
            flagD=flagD,
            th=(simulation_timeframe / 3600),
            spdata=[],
            genedata=[],
            sbml_file=self.sbml_file,
            model=perturbant_handler.model,
            f_genereg=self.f_genereg,
            f_omics=perturbant_handler.f_omics,
        )

        results = combine_results(perturbant_handler.model, xoutS_all, xoutG_all, tout_all)

        return results

    def run_amici_simulation(self, condition: pd.Series) -> tuple:
        """This function runs the AMICI simulation for a single condition.
        Parameters:
            self: Simulation - the Simulation class
            condition: pd.Series - the condition to simulate

        Returns:
            result: tuple - the simulation results

        """
                # Set the timepoints for the simulation
        simulation_timeframe = self.measurement_df["time"][
            self.measurement_df["simulationConditionId"].isin(condition)
        ].max()

        # Set the perturbations for the simulation
        perturbant_handler = Perturbations(self)
        
        perturbant_handler.set_perturbations(condition)

        xoutS_all, tout_all = RunAMICI(simulation_timeframe, perturbant_handler.model)

        results = combine_results(perturbant_handler.model, xoutS_all, None, tout_all)

        return results

    def preequilibrate(self, condition: pd.Series, flagD: bool) -> pd.DataFrame:
        """
        This function assigns a set of conditions that replicate
        prior experimental conditions before the primary stimulus of
        interest.

        Parameters:
        - condition (pd.Series): the condition to simulate
        - flagD (bool): the flag for if the simulation is deterministic or stochastic

        Returns:
        - preequilibrated_model (pd.DataFrame) the preequilibrated model
            OR 
        - model (pd.DataFrame): the model, sans preequilibration condition
        """
        # Isolate the preequilibration condition if included in the measurement
        # table
        if "preequilibrationConditionId" not in self.measurement_df.columns:
            return self.model
        
        preequilibrate_condition_id = (
            self.measurement_df.loc[
                self.measurement_df["simulationConditionId"]
                == condition["conditionId"],
                "preequilibrationConditionId",
            ]
            .dropna()
            .unique()
        )

        # account for no preequilibration condition being found
        if len(preequilibrate_condition_id) == 0:
            return self.model

        parent_condition = condition

        condition = self.conditions_df.loc[
            self.conditions_df["conditionId"] == preequilibrate_condition_id[0]
        ].iloc[0]

        # Set the perturbations for the simulation
        perturbant_handler = Perturbations(self)
        
        perturbant_handler.set_perturbations(condition)

        # Find the time frame for the preequilibration simulation
        simulation_timeframe = self.measurement_df["time"][
            self.measurement_df["simulationConditionId"] == preequilibrate_condition_id[0]
        ].max()

        print_statement = (f"simulating parent condition {parent_condition['conditionId']}",
                           f"preequilibration {preequilibrate_condition_id[0]}",
                           f"for {simulation_timeframe} seconds.")
        print(*print_statement)

        perturbant_handler.model.setTimepoints(np.linspace(0, 30))

        # Run the simulation
        xoutS_all, _, _ = RunSPARCED(
            flagD=flagD,
            th=(simulation_timeframe / 3600),
            spdata=[],
            genedata=[],
            sbml_file=self.sbml_file,
            model=perturbant_handler.model,
            f_genereg=self.f_genereg,
            f_omics=perturbant_handler.f_omics,
        )

        # Return the final values
        perturbant_handler.model.setInitialStates(xoutS_all[-1])

        return perturbant_handler.model
        
    def heterogenize(self, condition: pd.Series) -> libsbml.Model:
        """
        Runs the 'runSPARCED' function to simulate asynchrony among replicates.

        Parameters:
        - condition (pd.Series): the condition to simulate

        Returns:
        - model (libsbml.Model): the updated SBML model with stochastic heterogenization
        - model (libsbml.Model): the original SBML model if no heterogenization is required
        """
        heterogenize = condition.get("heterogenize_time", None)

        if heterogenize is None or isinstance(heterogenize, str) or (isinstance(heterogenize, float) and math.isnan(heterogenize)):
            return self.model  # Return original model if no heterogenization is required

        simulation_time = int(heterogenize) / 3600
        self.model.setTimepoints(np.linspace(0, 30))

        # TODO: Improve mechanism for setting signaling ligands to 0
        growth_factors = ["E", "H", "HGF", "P", "F", "I", "INS"]

        for species in growth_factors:
            self.model = Perturbations.set_species_value(self.model, species, 0)  # Set growth factors to 0

        xoutS_all, _, _ = RunSPARCED(
            flagD=0,  # Use stochastic solver for heterogenization
            th=simulation_time,
            spdata=[],
            genedata=[],
            sbml_file=self.sbml_file,
            model=self.model,
            f_genereg=self.f_genereg,
            f_omics=self.f_omics,
        )

        self.model.setInitialStates(xoutS_all[-1])

        return self.model

    def load_sparced_model(self):
        """
        This function loads the SPARCED model.

        Parameters:
            None

        Returns:
        - model (libsbml.Model): The SBML model
        """
        # Create an instance of the AMICI model.
        sys.path.append(self.sbml_file)

        utils.add_amici_path(self.sbml_file)

        sparced = utils.swig_interface_path(self.sbml_file)
        sys.path.append(sparced)
        SPARCED = importlib.import_module(sparced.split("/")[-1].split(".")[0])
        model = SPARCED.getModel()
        solver = model.getSolver()
        solver.setMaxSteps = 1e10

        return model

    @staticmethod
    def determine_hybrid_flag(condition: pd.Series) -> int:
        """
        Determines if hybrid is an included flag within the individual perturbation, 
        and if so, sets the model solver accordingly. 

        Parameters:
        - condition (pd.Series): key-value pair structure of perturbation identifiers
                                     (assigned following PEtab conditions table standard)
                                    matched to associated values. 

        Returns:
        - flagD (int): Deterministic decision flag for if the SPARCED model will solve 
                       the system deterministically, or use the integrated hybrid simulation 
                       setting, enabling stochastic trajectories.
        """

        if "hybrid" in condition.keys():
            if condition["hybrid"]:  # If hybrid is True
                flagD = 0
            
            else:
                flagD = 1

        else:
            flagD = 1

        return flagD
    
    @staticmethod
    def _remove_condition_name(conditions_df):
        """Remove 'conditionName' column from a DataFrame if it exists."""
        return conditions_df.drop(columns=['conditionName'], errors='ignore')
                
    @staticmethod
    def _extract_perturbations(conditions_df):
        """
        Takes conditions pandas series and returns each perturbation.

        Parameters:
        - condition (pd.Series): the individual simulation's set of perturbations
            to be applied prior to run-time.
        """
        condition_id_col = 'conditionId'
        return [x for x in conditions_df.columns if x != condition_id_col]

class Perturbations:
    """
    Handles differentiating between perturbation types (parameters, species, 
    and genes) and appropriately sets an updated value prior to simulation.
    Returns the updated object supplied.
    """
    def __init__(self, simulator: Simulator):
        """
        differentiates perturbations and applies a classifier function.
        
        Parameters:
        - perturbants (list): list of perturbants to be applied to model attributes.

        - condition (pd.Series): the individual simulation's set of perturbations
            to be applied prior to run-time. 
        """

        self.perturbants = simulator.perturbants
        self.model = simulator.model
        self.f_omics = simulator.f_omics
        self.sbml_file = simulator.sbml_file
        self.conditions_df = simulator.conditions_df

    def set_perturbations(self, condition):
        """Uses classified type to reassign model's value for perturbation."""

        perturbation_types = self._classify_perturbants()

        for perturbant in self.perturbants:

            if perturbant == "None":
                sys.exit('perturbation not found in model, exiting...')

            if perturbation_types[perturbant] == "species":
                self.model = self.set_species_value(
                    self.model, perturbant, condition[perturbant]
                )

            if perturbation_types[perturbant] == "parameter":
                # print("condition is :", condition)
                # print("dtype of condition@perturbant is", type(condition[perturbant]))
                self.model = self._set_parameter_value(
                    self.model, perturbant, condition[perturbant]
                )

            if perturbation_types[perturbant] == "gene":
                self.f_omics = self._set_transcription_values(
                    omics_data=self.f_omics,
                    gene=perturbant,
                    value=condition[perturbant],
                )

    def _classify_perturbants(self):
        """
        determines perturbation type (Species, Param, Gene, etc.)
        """
        perturbant_types = {} # Stores each perturbation's type

        for perturbant in self.perturbants:

            perturbant_types[perturbant] = self._check_entity_type(perturbant)

        return perturbant_types

    def _check_entity_type(self, entity_id):
        """Determine if entity_id is a species, parameter, or neither in an SBML model."""
        #load SBML model

        ignore_list = ['hybrid', 'heterogenize_time', 'cell_number']

        if entity_id in ignore_list:
            return None # These are SPARCED-specific settings that have custom class handlers

        model = self.load_sbml_model(self.sbml_file)

        # Check species
        if model.getSpecies(entity_id):
            return "species"
        
        # Check parameters
        if model.getParameter(entity_id):
            return "parameter"
        
        if self.f_omics is not None and entity_id in self.f_omics.index:
            return "gene"
        
        print(f"Model entity '{entity_id}' not found.")
        return "None"
        
    @staticmethod
    def _set_parameter_value(
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
        # print(f"Setting parameter: {parameter}, value: {parameter_value}, type: {type(parameter_value)}")

        try:  # assign the parameter value
            model.setParameterById(parameter, float(parameter_value))
        except RuntimeError:
            model.setFixedParameterById(parameter, float(parameter_value))

        return model

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
    def _set_transcription_values(
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
    def load_sbml_model(file_path):
        """Load an SBML model from a file."""
        reader = libsbml.SBMLReader()
        document = reader.readSBML(file_path)
        model = document.getModel()
        return model
        