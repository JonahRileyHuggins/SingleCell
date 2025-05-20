#!/bin/env python3 
# -*- coding: utf-8 -*-
"""
Author: JRH
Created: 05/02/2025
Description: 
"""

import os 

from types import SimpleNamespace

import yaml
import pandas as pd

class FileLoader:

    """Generic Object for loading files."""
    def __init__(self, yaml_path: str | os.PathLike):
        self.yaml_path = yaml_path
        self.model_files = self.__extract_model_build_files()
        del self.yaml_path
    
    def __load_yaml(self) -> dict:
        """Loads yaml file as object. """

        with open(self.yaml_path, encoding='utf-8', mode = 'r+') as file:
            yaml_dict = yaml.safe_load(file)

        return yaml_dict


    def __extract_model_build_files(self) -> SimpleNamespace:
        """returns input files as pandas dataframes, contained in an object for easy reference."""

        model_files = SimpleNamespace()

        yaml_dict = self.__load_yaml()

        yaml_dir = os.path.dirname(self.yaml_path)

        data_dir = os.path.join(yaml_dir, yaml_dict['compilation']['directory'])

        for key, value in yaml_dict['compilation']['files'].items():

            file_path = os.path.join(data_dir, value)

            setattr(model_files, key, pd.read_csv(file_path, sep = '\t', index_col=0, header=0))

        return model_files
