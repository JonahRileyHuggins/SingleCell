#!/bin/env python3 
# -*- coding: utf-8 -*-
"""
Author: JRH
Created: 05/02/2025
Description: Creates an instance of two sbml models for the genome-compelete MCF10A model.
"""

import os
import re
import logging
import argparse
from types import SimpleNamespace
from file_loader import FileLoader

import pandas as pd
import numpy as np

import libsbml
import amici
import antimony as sb

parser = argparse.ArgumentParser(prog='swap_name')
parser.add_argument('--yaml_path', '-p', default = None, help = 'path to configuration file detailing \
                                                                        which files to inspect for name changes.')
parser.add_argument('--catchall', '-c', metavar='KEY=VALUE', nargs='*',
                    help="Catch-all arguments passed as key=value pairs")
parser.add_argument('-v', '--verbose', help="Be verbose", action="store_true", dest="verbose"
)

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


class CreateModel:
    """Class for creating instances of an SBML model. Since there's going to be quite a bit of redundancy
    Between the two, this simplifies operation."""

    def __init__(self, model_name, args, **kwargs):

        logger.info('Starting build process for model %s ...', model_name)

        self.model_name = model_name

        self.sbml_doc = None
    
        self.sbml_model = None

        self.parameters = None

        self.model_files = FileLoader(args.yaml_path).model_files

        # Dont forget to write params file.
        # I'm just not for now... Later...
        
    def _convert_antimony_to_sbml(self):
        """Load antimony doc into an SBML object"""

        antimony_file_path = f'../sbml_files/antimony_{self.model_name}.txt'
        sbml_file_path = f'../sbml_files/{self.model_name}.sbml'

        if sb.loadFile(str(antimony_file_path)) == -1:
            logger.debug(sb.getLastError())
            raise RuntimeError(
            "Failed to load Antimony file of model " + f"{self.model_name}.\n"
        )

        logger.info(
                "SPARCED VERBOSE: Successfully loaded Antimony file of "
                + f"model {self.model_name}.\n"
            )
        
        # Convert the Antimony file into an SBML model
        if sb.writeSBMLFile(str(sbml_file_path), self.model_name) == 0:
            logger.debug(sb.getLastError())
            raise RuntimeError(
                "Failed to convert Antimony file of model "
                + f"{self.model_name} to SBML format.\n"
            )
        logger.debug(
                "SPARCED VERBOSE: Successfully converted Antimony file of "
                + f"model {self.model_name} to SBML format.\n"
            )
        return sbml_file_path

    def _load_sbml(self, sbml_file_path: str | os.PathLike):
        """Import the instance of our model."""
        logger.info('Loading SBML model %s')

        # create interaction components
        sbml_reader = libsbml.SBMLReader()
        self.sbml_doc = sbml_reader.readSBML(sbml_file_path)
        self.sbml_model = self.sbml_doc.getModel()

    def _add_annotations(self):
        """Appends annotations to the finalized sbml model"""

        # Set species annotations
        for speciesId, species_vals in self.model_files.species.iterrows():
            annotations = species_vals['annotation1':]
            Annot = ""

            for identifier in annotations:
                if pd.isna(identifier) or str(identifier).strip() == "":
                    break

                logger.debug('Species %s has annotation %s' % (speciesId, identifier))
                Annot = Annot + " " + str(identifier).strip()

            self.sbml_model.getSpecies(speciesId).setAnnotation(Annot.strip())
        
        # Set Compartment annotations
        for compartmentId, compartment_vals in self.model_files.compartments.iterrows():
            annotation = compartment_vals.get('annotation', '')
            if not pd.isna(annotation):
                self.sbml_model.getCompartment(compartmentId).setAnnotation(str(annotation).strip())

    def _write_sbml(self):
        writer = libsbml.SBMLWriter()
        writer.writeSBML(self.sbml_doc, self.sbml_file)

    @classmethod # Table, need method to build file handler.
    def factory_model_handler(self, model_name, args, **kwargs): 
        """Factory method for auto determining the type of model being built"""
        if model_name == 'Deterministic':
            DeterministicModel(model_name, args, **kwargs)
        else:
            StochasticModel(model_name, args, **kwargs)
            

class DeterministicModel(CreateModel):
    """Handles making the SBML and AMICI models from parent class CreateModel"""
    def __init__(self, model_name, args, **kwargs):
        super().__init__(model_name, args, **kwargs)

        # Place here the updated model files
        self._get_components()

        AntimonyFile(self.model_files, self.model_name, self.parameters)

        sbml_file_path = self._convert_antimony_to_sbml()

        self._load_sbml(sbml_file_path)

        self._add_annotations()

        self._make_AMICI_model(sbml_file_path)


    def _get_components(self):
        """Gets deterministic components only"""

        # Filter species for stochastic solver
        stochastic_params = self.model_files.species[
            self.model_files.species['solver'].str.lower().str.strip() == 'stochastic'
        ].reset_index()

        logger.info('>>>>>>> immediate parameters dataframe: %s' % (stochastic_params))

        # Create new DataFrame with desired columns
        self.parameters = stochastic_params[['speciesId', 'initialConcentration']].rename(
            columns={'speciesId': 'parameterId', 'initialConcentration': 'value'}
        )

        logger.info('>>>>>>>> params dataframe after column name: %s' % (self.parameters))

        self.model_files.species = self.model_files.species[
            self.model_files.species['solver'].str.lower().str.strip() == 'deterministic'
        ]
    
    def _make_AMICI_model(self, sbml_file_path):
        # Create an SbmlImporter instance for our SBML model


        amici_model_output_path = f'../amici_models/{self.model_name}'
    
        _make_output_dir(amici_model_output_path)


        sbml_importer = amici.SbmlImporter(sbml_file_path)

        constantParameters = [params.getId() for params in self.sbml_model.getListOfParameters()]



        # The actual compilation step by AMICI, takes a while to complete for large models
        sbml_importer.sbml2amici(self.model_name,
                                amici_model_output_path,
                                verbose=args.verbose,
                                constant_parameters=constantParameters)
        

class StochasticModel(CreateModel):
    """Handles making the SBML from parent class CreateModel"""
    def __init__(self, model_name, args, **kwargs):
        super().__init__(model_name, args, **kwargs)

        self._get_components()

        AntimonyFile(self.model_files, self.model_name, self.parameters)

        sbml_file_path = self._convert_antimony_to_sbml()

        self._load_sbml(sbml_file_path)

        self._add_annotations()

    def _get_components(self):
        """Gets stochastic components only, converts deterministic into parameters"""

        # Filter species for stochastic solver
        deterministic_params = self.model_files.species[
            self.model_files.species['solver'].str.lower().str.strip() == 'deterministic'
        ].reset_index()

        # Create new DataFrame with desired columns
        self.parameters = deterministic_params[['speciesId', 'initialConcentration']].rename(
            columns={'speciesId': 'parameterId', 'initialConcentration': 'value'}
        )

        self.model_files.species = self.model_files.species[
            self.model_files.species['solver'].str.lower().str.strip() == 'stochastic'
        ]

class AntimonyFile:
    """ Creates antimony file for easy conversion to SBML """
    def __init__(self, model_files: SimpleNamespace, model_name: str, parameters: pd.Series):
        self.model_files = model_files
        self.model_name = model_name
        ## Include other operations here. 
        self.parameters = parameters

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
        fileModel = open(f'../sbml_files/antimony_{self.model_name}.txt', encoding='utf-8', mode='w')
        logger.info('storing %s in ../sbml_files/antimony_%s' % (self.model_name, self.model_name))

        fileModel.write(f'# Genome-Complete {self.model_name} Model \n')
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

            ratelaw_info = Ratelaw.from_series(ratelaw_id, ratelaw_vals) # Cell 13, all the ridiculous reassigning lists.

            if ratelaw_info.reactants == [] and ratelaw_info.products == []:
                continue

            self.antimony_file.write( # bottom of Cell 13
                f"  {ratelaw_id}: "
                + f"{' + '.join(ratelaw_info.reactants)} => {' + '.join(ratelaw_info.products)}; "
                + f"({ratelaw_info.formula})*{ratelaw_info.compartment};\n"
            )
            logger.info("Formula %s for Ratelaw %s written to antimony document." % (ratelaw_info.formula, ratelaw_id))

            self.parameters = pd.concat([self.parameters, pd.DataFrame(ratelaw_info.parameters)])

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

            self.antimony_file.write("  %s = %.6e;\n" % (species_name, np.double(species_vals['initialConcentration'])))

            logger.info("Assigning Species %s equal to %.6e;\n" % ((species_name, np.double(species_vals['initialConcentration']))))

    def __assign_parameter_initial_concentrations(self): # Cell 22
        """Write the parameters of every reaction to antimony document"""

        logger.info('>>> Starting Parameter Assignments')

        # Write parameter ICs
        self.antimony_file.write("\n  # Parameter initializations:\n")

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


class Ratelaw:
    """Composite Class of AntimonyFile, separating reaction differences without gratuitous if/else statements"""

    def __init__(self, reactionId: str, ratelaw: pd.Series):
        self.reactionId = reactionId
        self.ratelaw = ratelaw

        self.formula = None
        self.parameters = {'parameterId': [], 'value': []}
        self.reactants = None
        self.products = None
        self.compartment = ratelaw['compartment']

        self.__get_reactants_products()

    def __get_reactants_products(self):
        """Parses reactants and products from 'r ; p' string in ratelaw row."""
        rxn = self.ratelaw.get('r ; p', '')

        if pd.isna(rxn) or not isinstance(rxn, str):
            logger.debug("Reaction entry is NaN or not a string: %s", rxn)
            return [], []

        reactants_str, products_str = (rxn.split(';') + [''])[:2]
        logger.debug("Parsed reaction string: reactants='%s', products='%s'", reactants_str, products_str)

        self.reactants = [r.strip() for r in reactants_str.split('+') if r.strip()]
        self.products = [p.strip() for p in products_str.split('+') if p.strip()]

        logger.debug("Final parsed lists: reactants=%s, products=%s", self.reactants, self.products)

    def __build_rxn_formula(self):
        """extracts the formula """
        raise NotImplementedError("Method is required for subclass.")

    @classmethod
    def from_series(cls, reactionId: str, ratelaw:pd.Series):
        """Factory method for auto determining the type of reaction at hand"""
        formula = ratelaw.get('ratelaw', None)

        if isinstance(formula, (float, int)): # this would be changed with new tables
            return MassAction(reactionId, ratelaw)
        return NthOrder(reactionId, ratelaw)


class MassAction(Ratelaw):
    """For setting up mass action formulas"""

    def __init__(self, reactionId, ratelaw):
        super().__init__(reactionId, ratelaw)

        self.__build_rxn_formula()

        del self.ratelaw
        del self.reactionId

    def __build_rxn_formula(self):
        """First order ratelaws don't have a formula in the column, only an float64 value."""
        self.formula = "k"+str(self.reactionId)+'_1'

        self.parameters['parameterId'].append(self.formula)
        self.parameters['value'].append(self.ratelaw['ratelaw'])


class NthOrder(Ratelaw):
    """For setting up ratelaws of First through Nth order. Determined in Ratelaw."""

    def __init__(self, reactionId, ratelaw):
        super().__init__(reactionId, ratelaw)

        self.__build_rxn_formula()

        del self.ratelaw
        del self.reactionId

    def __build_rxn_formula(self):
        """builds formula for non-mass-action ratelaws."""

        self.formula = self.ratelaw['ratelaw']

        parameter_prefix = f'k{self.reactionId[1:]}_'

        parameter_columns = self.ratelaw.loc['parameter1':]

        parameter_columns = parameter_columns[parameter_columns.notna()].index.tolist()
        
        j = 1

        for parameter in parameter_columns:

            value = self.ratelaw[parameter]

            pattern = parameter_prefix+str(j)

            match = re.search(pattern, self.formula)

            if match:

                self.parameters['parameterId'].append(pattern)
                self.parameters['value'].append(value)

                logger.info('Parameter %s assigned value %s' % (pattern, value))

            else:
                logger.debug('Parameter %s not found despite %s number of parameter values. Inspect Ratelaw Table at %s' % (
                    pattern, len(parameter_columns), self.reactionId 
                ))
            j +=1


@staticmethod
def _make_output_dir(amici_model_path: str | os.PathLike) -> None:
    """ Provide a path and this returns a directory. Separating from Classes for operability."""
    if not os.path.exists(amici_model_path):
        os.mkdir(path=amici_model_path)

@staticmethod
def parse_kwargs(arg_list: list)-> dict:
    """Parses catchall function."""


    kwargs = {}


    for arg in arg_list:
        if '=' not in arg:
            raise ValueError(f"Invalid argument format: {arg}. Use key=value.")
        else:
            key, value = arg.split('=', 1)
            kwargs[key] = value


    return kwargs

if __name__ == '__main__':

    args = parser.parse_args()
   
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    kwargs = parse_kwargs(args.catchall) if args.catchall else {}


    deterministic_model_name = 'Deterministic'
    stochastic_model_name = 'Stochastic'

    CreateModel.factory_model_handler(deterministic_model_name, args, **kwargs)

    CreateModel.factory_model_handler(stochastic_model_name, args, **kwargs)

