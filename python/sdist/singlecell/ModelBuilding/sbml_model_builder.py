#!/bin/env python3 
# -*- coding: utf-8 -*-
"""
Author: JRH
Created: 05/02/2025
Description: Creates an instance of two sbml models for the genome-compelete MCF10A model.
"""

import os
import logging
from types import SimpleNamespace

from singlecell.shared_utils.utils import parse_kwargs

import pandas as pd

import libsbml
import antimony as sb

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

class CreateSBMLModel:
    """Class for creating instances of an SBML model. Since there's going to be quite a bit of redundancy
    Between the two, this simplifies operation."""

    def __init__(
            self, 
            solver: str, 
            solver_components: SimpleNamespace,
            antimony_output_dir: os.PathLike | str, 
            sbml_output_dir: os.PathLike | str
            ):

        logger.info('Starting build process for model %s ...', solver)

        self.sbml_doc = None
    
        self.sbml_model = None

        self.model_name = solver

        self.antimony_output_dir = antimony_output_dir
        
        self.sbml_output_dir = sbml_output_dir
        
        sbml_file_path = self.__convert_antimony_to_sbml()

        self._load_sbml(sbml_file_path)

        self._add_annotations(solver_components)

        # Clean up, clean up, everybody do your share...
        del self.sbml_doc
        del self.sbml_model
        del self.model_name


    def __convert_antimony_to_sbml(self):
        """Load antimony doc into an SBML object"""

        antimony_file_path = f'{self.antimony_output_dir}/antimony_{self.model_name}.txt'
        sbml_file_path = f'{self.sbml_output_dir}/{self.model_name}.xml'

        if sb.loadFile(str(antimony_file_path)) == -1:
            logger.debug(sb.getLastError())
            raise RuntimeError(
            "Failed to load Antimony file of model " + f"{self.model_name}.\n"
        )

        logger.info(
                "Successfully loaded Antimony file of "
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
                "Successfully converted Antimony file of "
                + f"model {self.model_name} to SBML format.\n"
            )
        return sbml_file_path


    def _load_sbml(self, sbml_file_path: str | os.PathLike):
        """Import the instance of our model."""
        logger.info('Loading SBML model %s', self.model_name+'.sbml')

        # create interaction components
        sbml_reader = libsbml.SBMLReader()
        self.sbml_doc = sbml_reader.readSBML(sbml_file_path)
        self.sbml_model = self.sbml_doc.getModel()


    def _add_annotations(
            self, 
            solver_components: SimpleNamespace
            ):
        """Appends annotations to the finalized sbml model"""

        # Set species annotations
        for speciesId, species_vals in solver_components.species.iterrows():
            annotations = species_vals['annotation1':]
            Annot = ""

            for identifier in annotations:
                if pd.isna(identifier) or str(identifier).strip() == "":
                    break

                logger.debug('Species %s has annotation %s' % (speciesId, identifier))
                Annot = Annot + " " + str(identifier).strip()

            self.sbml_model.getSpecies(speciesId).setAnnotation(Annot.strip())
        
        # Set Compartment annotations
        for compartmentId, compartment_vals in solver_components.compartments.iterrows():
            annotation = compartment_vals.get('annotation', '')
            if not pd.isna(annotation):
                self.sbml_model.getCompartment(compartmentId).setAnnotation(str(annotation).strip())

        self._write_sbml()


    def _write_sbml(self):
        writer = libsbml.SBMLWriter()

        sbml_output_path = f'{self.output_path}/{self.model_name}.xml'

        writer.writeSBML(self.sbml_doc, sbml_output_path)

