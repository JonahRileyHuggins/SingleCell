#!/bin/env python3 
# -*- coding: utf-8 -*-
"""
Author: JRH
Created: 05/02/2025
Description: Prints to screen any identified species in a particular file that does not have a
match in the Species model build file.
"""

import os
import re
import logging
import argparse
import pandas as pd


from file_loader import Config


logging.basicConfig(
    level=logging.INFO,  # This will be overridden by setLevel() if verbose is True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


script_dir = os.path.dirname(os.path.abspath(__file__))


parser = argparse.ArgumentParser(prog='incorrect-inspector')
parser.add_argument('--path', '-p', default = None, help = 'path to configuration file detailing \
                                                                        which files to inspect for name changes.')
parser.add_argument('--catchall', '-c', metavar='KEY=VALUE', nargs='*',
                    help="Catch-all arguments passed as key=value pairs")
parser.add_argument('--output', '-o', metavar='OUTPUT', help = 'desired path to return list of old names from inspection, compliant with ' \
                    'format for species name converter script. default column names are old')
parser.add_argument('-v', '--verbose', help="Be verbose", action="store_true", dest="verbose"
)

parameter_regex = re.compile(
    'k([A-Z]{1}[a-zA-Z0-9]*[0-9]+[a-z]*(_[0-9]+)+)'
    )

species_regex = re.compile(
    r"^[a-z]{3}_(prot_|lipid_|mrna_|gene_|mixed_|imp_)(([a-zA-Z]+)_)*(((_[a-z]{1}[A-Z]?[0-9]*)+)*(_[a-zA-Z0-9]+_([0-9_]*)))+$"
)


def main(config_path: os.PathLike, args, **kwargs) -> None:
    """
    Checks files in inspect section of config against reference species list.
    """
    logger.info("Starting isolation process using config: %s", config_path)

    config = Config.file_loader(config_path)

    project_root = "./../" # Single cell root directory

    config_base = os.path.join(project_root, config.get("location", ""))

    logger.debug("Loaded configuration successfully")

    inspector = config.get("inspector", {})
    reference_path = os.path.join(config_base, inspector.get("reference", {}).get("filepath", {}))

    reference_file = Config.file_loader(reference_path)
    logger.debug("Reference file %s successfully loaded", reference_path)

    # Load the reference species set
    reference_column = reference_file[config.inspector.reference.column]
    reference_species_set = set(reference_column.dropna().unique())
    logger.info("Loaded %d unique reference species", len(reference_species_set))

    files_to_inspect = config.get("inspector", {}).get("inspect", {})

    logger.info("Found %s file to investigate", len(files_to_inspect.keys()))

    if args.output:
        output_file = pd.DataFrame(columns=['old'])

    for index, f in enumerate(files_to_inspect):
        logger.info("Processing file [%d/%d]: %s", index + 1, len(files_to_inspect), files_to_inspect[f]['path'])
        
        file_path = os.path.join(config_base, files_to_inspect[f]['path'])
        inspect_me = Config.file_loader(file_path, **kwargs)
        logger.debug("Loaded file successfully: %s", files_to_inspect[f]['path'])

        # Each column to inspect (except 'filepath' key)
        inspect_column_ids = [key for key in config.inspector.inspect[f] if key != 'path']

        for column in inspect_column_ids:
            col_id = config.inspector.inspect[f][column]

            if col_id not in inspect_me:
                logger.warning("Column %s not found in file %s", col_id, f)
                continue

            logger.info("Inspecting column: %s", col_id)

            species_found = inspect_me[col_id]
            for index, entry in enumerate(species_found):

                # Split on math operators
                tokens = re.split(r'([+\-*/^();])', entry)
                
                # Remove whitespace
                tokens = [t.strip() for t in tokens if t.strip()]

                # Remove parameters
                tokens = [t for t in tokens if not parameter_regex.fullmatch(t)]

                operators = set("([+-*/^();])")  # Using a set for efficient checking

                tokens = [s for s in tokens if not any(op in s for op in operators)]

                for token in tokens:
                    if not species_regex.match(str(token)):
                        logger.warning("Token in %s doesn't match species pattern: %s", col_id, token)
                        if args.output:
                            output_file = pd.concat([output_file, pd.DataFrame({'old': [token]})])
                        continue
                    if token not in reference_species_set:
                        print(f"[MISSING] '{token}' in column '{col_id}' index '{index}' of file '{config.inspector.inspect[f]['path']}' not found in reference list.")
                        if args.output:
                            output_file = pd.concat([output_file, pd.DataFrame({'old': [token]})])

    if args.output:
        output_file = output_file.drop_duplicates()
        output_file = output_file.dropna()
        # output_file = output_file.unique()
        output_file.to_csv(args.output, index=False, sep = '\t')


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


    main(args.path, args, **kwargs)
