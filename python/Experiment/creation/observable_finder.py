#!/usr/bin/env python     
# -*- coding: utf-8 -*-
"""
script name: observable_finder.py
Created on Fri Jan. 17th 2025
Author: Jonah R. Huggins

Description: This script generates an Observable formula for a given species in an SBML model.
            The goal is to take a particular species name or annotation number and generate the summated 
            formula for the observable, including the compartment volume ratio.

"""
# -----------------------Package Import & Defined Arguements-------------------#
import os
import re
import argparse
import yaml
import pandas as pd

# Parse the command line arguments
parser = argparse.ArgumentParser(description='Generate the observable formula for a given observable')
parser.add_argument('--input', '-i', type=str, nargs='+', required=True, help='The input species or annotation number')
parser.add_argument('--yaml_path', '-p', default = None, help = 'path to configuration file detailing \
                                                                        which files to inspect for name changes.')
args = parser.parse_args()


class SpeciesRules:
    """
    Defines the rules for species strings based on components.
    """

    species_regex = re.compile(
    r"^[a-z]{3}_(prot_|lipid_|mrna_|gene_|mixed_|imp_)(([a-zA-Z]+)_)*(((_[a-z]{1}[A-Z]?[0-9]*)+)*(_[a-zA-Z0-9]+_([0-9_]*)))+$"
    )

    COMPARTMENTS = {
        '_cyt_': ['Cytoplasm', 'cyt_', 'Cytosol'],
        '_mit_': ['Mitochondria', 'Mitochondrion', 'mit_'],
        '_nuc_': ['Nucleus', 'nuc_'],
        '_end_': ['Endosome', 'end_'], 
        '_exc_': ['Extracellular', 'exc_'],
    }

    SPECIES_TYPES = {
        '_mrna_': ['mRNA', '_mrna_'],
        '_prot_': ['Protein', '_prot_'],
        '_lipid_': ['Lipid', '_lipid_'],
        '_cong_': ['Conglomerate', '_cong_'],
        '_entity_': ['Entity', '_entity_'],
    }

    MODIFIERS = {
        '_p_': ['Phosphorylated', '_p_'],
        '_u_': ['Ubiquitinated', '_u_'],
        '_cl_': ['Cleaved', '_cl_'],
        '_a_': ['Activated', '_a_'],
        '_i_': ['Inactivated', '_i_'],
    }

    CORE = r'_[a-zA-Z][\w]*_'

    STRUCTURE = r'^(?P<compartment>cyt_|mit_|nuc_|end_|exc_)' \
                r'(?P<species_type>mrna_|prot_|lipid_|cong_|entity_)' \
                r'(?P<modifiers>(_p|_u|_cl|_a|_i)*)' \
                r'(?P<residue_position>_\d+[A-Z]?)?' \
                r'_(?P<base_species>[A-Za-z0-9]+)' \
                r'(__(?P<additional_species>[A-Za-z0-9]+))?$'

    def __init__(self, string: str)-> None:
        """
        validates the species string provided by the user.
        """
        self.type = self.validate_species_string(string)

    @classmethod
    def validate_species_string(cls, species_string):
        """
        Validates the given species string against the defined patterns. This is iterative, 
        so that if a user inputs any constant above, it will be validated.
        
        :param species_string: The species string to validate.
        :return: True if valid, False otherwise.
        """
        # First attempt a match against the full structure
        match = re.match(cls.STRUCTURE, species_string)
        if match:
            return 'full_structure'
        
        # Since full string not matched, ensure underscores are added to match to const component keys
        species_string = cls.add_underscores(species_string)

        # If full structure fails, try to match against the core name
        match = re.match(cls.CORE, species_string)
        if match:
            return 'is species'

        # If not a species, check if input wants all of a compartment type
        for comp_type, name_opts in cls.COMPARTMENTS.items():
            for name in name_opts:
                name = cls.add_underscores(name)
                if species_string in name:
                    return f'is {comp_type}'
                
        # If not a compartment, check if input wants all of a species type
        for spec_type, name_opts in cls.SPECIES_TYPES.items():
            for name in name_opts:
                name = cls.add_underscores(name)
                if species_string in name:
                    return f'is {spec_type}'
                
        # If not a species type, check if input wants all of a modifier
        for mod_type, name_opts in cls.MODIFIERS.items():
            for name in name_opts:
                name = cls.add_underscores(name)
                if species_string in name:
                    return f'is {mod_type}'
                
        return 'is not a species'

    @staticmethod
    def add_underscores(string):
        """
        Checks if the string provided in validate_species_string has underscores.
        if not, it will add them.
        """
        if string[0] != '_':
            string = '_' + string

        if string[-1] != '_':
            string = string + '_'

        return string


class UserInput:
    """
    Validates the user input for the species or annotation number.
    """

    def __init__(self, input_string: str, yaml_path: str):
        """
        Steps of User Input:
        1. Determine if input string is a species or annotation number.
        2. validate the string against the SpeciesRules structure, if species.
        3. Return the string with identification of species or annotation number.
        """
        self.yaml_path = yaml_path
        self.input_string = input_string

        self.config = self.load_config()

        self.input_components = self.parse_input(self.input_string)


    def parse_input(self, *args) -> list:
        """
        Parse the input components from *args. 
        - Supports both space- or comma-separated strings.
        """
        input_components = []
        
        for arg in args:
            if isinstance(arg, list):
                # Flatten list by extneding it into input_components
                input_components.extend(arg)

            elif isinstance(arg, str):
                # Handle comma- or space-separated strings
                input_components.extend(arg.replace(',', ' ').split())

            else:
                raise ValueError(f"Unsupported input type: {type(arg)}. Expected str or list.")
            
            return input_components
        
        input_string = ' '.join(args)
        input_string = input_string.replace(',', ' ')
        input_string = input_string.split()
        return input_string
    
    def load_config(self) -> dict:
        """
        Load the YAML configuration file.
        """
        try: 
            with open(self.yaml_path, encoding='utf-8', mode='r') as file:
                config = yaml.load(file, Loader=yaml.FullLoader)
            return config
        except FileNotFoundError(f'Could not find the file {self.yaml_path}') as e:
            print(e)
            return None


class SpeciesQuery:
    """
    Loads an instance of either the model input files or the SBML model file.
    Takes user defined input and returns the species that match the input components.
    """

    def __init__(self, input_string: str, yaml_path: str):
        """
        Initialize the model path and load the SBML model.
        """
        self.user_input = UserInput(input_string, yaml_path)

        self.component_types = {}
        for component in self.user_input.input_components:
            self.component_types[component] = SpeciesRules(component).type

        self.model_files = self.load_model_files()

        # Find species inside the input files that match all components in the input_components list

    def load_model_files(self):
        """
        Load files used to build the model from the configuration file.

        Parameters:
            config: dict: The configuration file.

        Returns:
            dict: The model files.
        """
        data_files = os.path.join(os.path.dirname(self.user_input.yaml_path), 
                                   self.user_input.config['location'])

        compilation_files = os.path.join(data_files, self.user_input.config['compilation']['directory'])

        species_path = os.path.join(compilation_files, self.user_input.config['compilation']['files']['species'])

        compartments_path = os.path.join(compilation_files, self.user_input.config['compilation']['files']['compartments'])

        ratelaws_path = os.path.join(compilation_files, self.user_input.config['compilation']['files']['ratelaws'])

        species_file = pd.read_csv(species_path, sep='\t')

        compartments_file = pd.read_csv(compartments_path, sep='\t')

        ratelaws_file = pd.read_csv(ratelaws_path, sep='\t')

        return {
            'species': species_file,
            'compartments': compartments_file,
            'ratelaws': ratelaws_file,
        }
    
    def get_species_given_component(self, component: str) -> list:
        """
        Takes a species name and returns the list of species that match the core component.

        Parameters:
        - component: str: The core component of a species name (e.g. '_JAK_', '_EGFR_', etc.)

        Returns:
        - list: The list of 
        """
        component = SpeciesRules.add_underscores(component)
        
        species_df = self.model_files['species']

        species_df['species'] = species_df['species'].astype(str)
        matching_species = species_df.loc[
        species_df['species'].str.contains(re.escape(component), na=False), 'species'
        ]
        return matching_species.tolist()


    def get_species_given_annotation(self, annotation: str):
        """
        Takes species name and 
        
        Parameters:
        - annotation: str: The input annotation number.
            
        Returns:
        - list: The related species.
        """
        related_species = []
        for index, row in self.model_files['species'].iterrows():
            if annotation in row['annotation']:
                related_species.append(row['species'])

        return related_species

    def __call__(self):
        """
        Finds and returns species that match all components in the input_components list.
        """
        # Start with a set of all species
        all_species_sets = []

        for component in self.user_input.input_components:
            if self.component_types[component] == 'is species':
                species_set = set(self.get_species_given_component(component))
            elif self.component_types[component] == 'is not a species':
                species_set = set(self.get_species_given_annotation(component))
            else:
                species_set = set(self.get_species_given_component(component))

            all_species_sets.append(species_set)

        # Find the intersection of all species sets
        if all_species_sets:
            matching_species = set.intersection(*all_species_sets)
        else:
            matching_species = set()

        return list(matching_species)
     
class ObservableBuilder:
    """
    Takes the species that match the input components and builds the observable formula.
    """
    
    def __init__(self, input_string: str, yaml_path: str):
        """
        Initialize the SpeciesQuery instance and load the species that match the input components.
        """
        self.species_query = SpeciesQuery(input_string, yaml_path)
        self.species = self.species_query()

    def build_observable(self):
        """
        Build the observable formula for the given species.
        """

        comp_volumes = self.extract_compartmental_volume()

        observable = ''

        for specie in self.species:

            specie_comp = self.get_species_compartment_volume(specie=specie)

            comp_volume = comp_volumes[specie_comp]

            num_instances = self.get_instance_of_component_in_species(queried_specie=specie)

            observable += f'({num_instances} * {specie} * {comp_volume}) + '

        return observable[:-3]
    
    def extract_compartmental_volume(self):
        """
        finds the volume of each compartment in the comparmtents file
        Returns:
        - dict: The compartmental volumes.
        """
        compartments = self.species_query.model_files['compartments']
        compartments = compartments.set_index('compartments')

        compartment_volumes = {}

        for index, row in compartments.iterrows():
            compartment_volumes[index] = row['volume']

        return compartment_volumes
    
    def get_species_compartment_volume(self, specie: str):
        """
        Takes a particular species in the self.species list and returns the 
        corresponding compartmental volume using the species file.

        Returns:
        - dict: The species compartmental ratio.
        """
        species_sheet = self.species_query.model_files['species']

        specie_row = species_sheet.loc[species_sheet['species'] == specie]

        specie_comp = specie_row['compartment'].values[0]

        return specie_comp

    def get_instance_of_component_in_species(self, queried_specie: str):
        """
        We need to know how many times a user-provided component appears in each species name. 
        This function finds the number of times a component appears in each species name and returns a 
        integer value.

        Parameters:
        - queried_specie: str: The species name to query.

        Returns:
        - num_instances: int: The number of times the component appears in the species name.
        """

        num_instances = 0

        for component in self.species_query.user_input.input_components:

            component = SpeciesRules.add_underscores(component)
            
            num_instances += queried_specie.count(component)

        return num_instances


    def __call__(self):
        """
        Build the observable formula for the given species.
        """
        return self.build_observable()


if __name__ == '__main__':
    observable = ObservableBuilder(args.input, args.yaml_path)
    print(observable())
    