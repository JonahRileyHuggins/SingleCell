#!/bin/env python3 
# -*- coding: utf-8 -*-
"""
Author: JRH
Created: 05/24/2025
Description: Creates an instance of two antimony models 
for the genome-compelete MCF10A model.
"""
import os
import logging
from types import SimpleNamespace

import pandas as pd
import numpy as np

from singlecell.ModelBuilding.ratelaw_handler import Ratelaw

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

class CreateAntimonyFile:
    """ Creates antimony file for easy conversion to SBML """
    def __init__(
        self, 
        solver_components: SimpleNamespace, 
        model_name: str,
        output: os.PathLike | str
    ) -> None:
        self.model_files = solver_components
        self.model_name = model_name
        ## Include other operations here. 
        self.parameters = solver_components.other_params
        self.output = output

        self.antimony_file = self.__create_antimony_file()

        self.__write_compartments()

        self.__write_species()

        self.__write_reactions()

        self.__assign_compartment_initial_concentrations()

        self.__assign_species_initial_concentrations()

        self.__assign_parameter_initial_concentrations()

        self.__make_compartments_constant()

        self.__assign_units()

        self.__end_antimony_file()

        # Don't create redundancies:
        del self.model_files
        del self.model_name

    def __create_antimony_file(self): #step 1, handled in cell 4
        """Creates a variable to store the antimony file document. Header started, 
        returned during init stage fore OOP process. """
        fileModel = open(f'{self.output}/antimony_{self.model_name}.txt', encoding='utf-8', mode='w')
        logger.info('storing %s in %s/antimony_%s' % (self.model_name, self.output, self.model_name))

        fileModel.write(f'# Human Epithelial Cell Model -- {self.model_name} \n')
        fileModel.write(f'model {self.model_name}()\n')
        return fileModel

    def __write_compartments(self): # step 2, handled in cells 6 & 7.
        """Update the antimony file with compartments"""
        logger.info('Writing Compartments to antimony document %s', self.model_name)
        
        compartment_names = self.model_files.compartments.index.to_list()

        self.antimony_file.write("\n  # Compartments and Species:\n") # Antimony Compartments/Species module title

        for name in compartment_names:
            self.antimony_file.write("  Compartment %s;\n" % (name))
            logger.info('Compartment "%s" written to antimony document', name)
        self.antimony_file.write('\n') 
            
    def __write_species(self): #step 3
        """Write species in input tables to antimony files"""
        logger.info("Writing species to Antimony document %s", self.model_name)
        self.antimony_file.write("\n")

        species_df = self.model_files.species # handled in cell 8
        
        for speciesid, species_vals in species_df.iterrows():
            species_compartment = species_vals['compartment'] # handled in cell 9
            
            self.antimony_file.write("  Species ") #handled in cell 10
            self.antimony_file.write("%s in %s" % (speciesid, species_compartment))
            self.antimony_file.write(';\n')

            logger.info("Species '%s' in compartment '%s' writen to antimony document" % (speciesid, species_compartment))

    def __write_reactions(self): #handled in cells 12 & 13
        """Writes given reactions to antimony file."""
        logger.info("Writing ratelaws to antimony document %s", self.model_name)

        self.antimony_file.write("\n\n # Reactions:\n") # Cell 12, Line 3

        ratelaws_df = self.model_files.ratelaws

        for ratelaw_id, ratelaw_vals in ratelaws_df.iterrows():

            ratelaw_info = Ratelaw(ratelaw_id, ratelaw_vals) # Cell 13, all the ridiculous reassigning lists.

            if ratelaw_info.reactants == [] and ratelaw_info.products == []:
                continue

            self.antimony_file.write( # bottom of Cell 13
                f"  {ratelaw_id}: "
                + f"{' + '.join(ratelaw_info.reactants)} => {' + '.join(ratelaw_info.products)}; "
                + f"({ratelaw_info.formula})"
            )
            self.antimony_file.write(
                f"*{ratelaw_info.compartment};\n"
            )

            logger.info("Formula %s for Ratelaw %s written to antimony document." % (ratelaw_info.formula, ratelaw_id))

    def __assign_compartment_initial_concentrations(self): # Cell 20
        """Write compartmental initial concentrations to antimony document"""

        compartments_df = self.model_files.compartments

        # Write compartment ICs
        self.antimony_file.write("\n  # Compartment initializations:\n")

        for index, (compartment_name, compartment_vals) in enumerate(compartments_df.iterrows()):

            self.antimony_file.write("  %s = %.6e;\n" % (compartment_name, np.double(compartment_vals['volume'])))

            self.antimony_file.write("  %s has volume;\n" % (compartment_name))
            
            logger.info("Compartment %s has volume %s " % (compartment_name, np.double(compartment_vals['volume'])))
 
    def __assign_species_initial_concentrations(self): # Cell 21
        """Write species initial concentrations to antimony document"""

        species_df = self.model_files.species

        self.antimony_file.write("\n  # Species initializations:\n")

        for index, (species_name, species_vals) in enumerate(species_df.iterrows()):

            self.antimony_file.write("  %s = %.6e;\n" % (species_name, np.double(species_vals['initialConcentration (nM)'])))

            logger.info("Assigning Species %s equal to %.6e;\n" % ((species_name, np.double(species_vals['initialConcentration (nM)']))))

    def __update_parameters(self) -> None:
        """getter method for making parameters object, intended only for use by antimonyModel

        Returns:
            None: assigns parameters table values to self.parameters object.
        """
        self.model_files.parameters = self.model_files.parameters.reset_index()

        params_include_df = self.model_files.parameters[['parameterId', 'nominalValue']].copy()

        params_include_df.rename(columns={'nominalValue':"value"}, inplace = True)

        self.parameters = pd.concat([self.parameters, params_include_df])

        return None

    def __assign_parameter_initial_concentrations(self): # Cell 22
        """Write the parameters of every reaction to antimony document"""

        logger.info('>>> Starting Parameter Assignments')

        # Write parameter ICs
        self.antimony_file.write("\n  # Parameter initializations:\n")

        self.__update_parameters()

        for idx, param_vals in self.parameters.iterrows():

            self.antimony_file.write("  %s = %.6e;\n" % (param_vals['parameterId'], np.double(param_vals['value'])))

            logger.info("Assigned Parameter %s value %s" % (param_vals['parameterId'], np.double(param_vals['value'])))

    def __make_compartments_constant(self):
        """Write compartments as constants"""
        const_compartments = list(self.model_files.compartments.index.to_list())

        self.antimony_file.write("\n  # Other declarations:\n")
        self.antimony_file.write("  const ")

        # Join all compartment names with commas, then end with semicolon and newline
        compartment_line = ",".join(const_compartments) + ";\n"
        self.antimony_file.write(compartment_line)

    def __assign_units(self):
        """Writing Model Units"""

        # Write unit definitions
        self.antimony_file.write("\n  # Unit definitions:")
        self.antimony_file.write("\n  unit time_unit = second;")
        self.antimony_file.write("\n  unit volume = litre;")
        self.antimony_file.write("\n  unit substance = 1e-9 mole;")
        self.antimony_file.write("\n  unit nM = 1e-9 mole / litre;")
        self.antimony_file.write("\n")

    def __end_antimony_file(self):
        """write the bottom of the file and close document"""

        # End the model file
        self.antimony_file.write("\nend")
        # Close the file
        self.antimony_file.close()
