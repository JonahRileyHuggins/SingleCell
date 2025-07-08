#!/bin/env python3
"""
filename: species_name_converter.py




description: takes old array of names, either a file path or a series of files organized
in a yaml file and swaps the names from old to species conforming.


! Assumption: Indexes for old and new columns are appropriately ordered and matched.
! (i.e. first entry in old is intended swap recipient for new name.)
"""


import os
import re
import logging
import argparse


from file_loader import Config


logging.basicConfig(
    level=logging.INFO,  # This will be overridden by setLevel() if verbose is True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


script_dir = os.path.dirname(os.path.abspath(__file__))


parser = argparse.ArgumentParser(prog='swap_name')
parser.add_argument('--path', '-p', default = None, help = 'path to configuration file detailing \
                                                                        which files to inspect for name changes.')
parser.add_argument('--catchall', '-c', metavar='KEY=VALUE', nargs='*',
                    help="Catch-all arguments passed as key=value pairs")
parser.add_argument('-v', '--verbose', help="Be verbose", action="store_true", dest="verbose"
)


def main(config_path: os.PathLike, **kwargs) -> None:
    """
    Swaps file names in method parameters args with new names
    """
    logger.info("Starting name swap process using config: %s", config_path)


    config = Config.file_loader(config_path)
    logger.debug("Loaded configuration successfully")


    
    project_root = "./../" # Single cell root directory
    config_base = os.path.join(project_root, config.get("location", ""))


    name_map = handle_mapping(config, **kwargs)
    logger.info("Name mapping constructed with %d entries", len(name_map))


    files_to_update = config.get("swap_files", {}).get("update", {})
    file_paths = [file_key["filename"] for file_key in files_to_update]


    logger.info("Found %d files to update", len(file_paths))


    for index, file in enumerate(file_paths):
        logger.info("Processing file [%d/%d]: %s", index + 1, len(file_paths), file)


        update_path = os.path.join(config_base, file)

        update_me = Config.file_loader(update_path, **kwargs)
        logger.debug("Loaded file successfully: %s", file)


        updated = update_me.map(lambda cell: replace_names(cell, name_map))
        logger.debug("Replaced species names in file: %s", file)


        output_path = files_to_update[index]['output']
        updated.to_csv(output_path, **kwargs)
        logger.info("Saved updated file to: %s", output_path)


    logger.info("All files processed successfully.")


def handle_mapping(config: dict, **kwargs) -> dict:
    """Takes the config file and returns dictionary with properly formatted key-value pairs"""
    logger.debug("Loading old names from: %s", config.swap_files.old.filename)
    old_file = Config.file_loader(config.swap_files.old.filename, **kwargs)
    old_names = old_file[config.swap_files.old.column].dropna()


    logger.debug("Loading new names from: %s", config.swap_files.new.filename)
    new_file = Config.file_loader(config.swap_files.new.filename, **kwargs)
    new_names = new_file[config.swap_files.new.column][:len(old_names)]


    name_map = dict(zip(list(old_names), list(new_names)))
    logger.debug("Constructed name map with %d entries", len(name_map))


    return name_map


def replace_names(expr: str, name_map: dict) -> str:
    if not isinstance(expr, str):
        return expr
    # Regex to match scientific notation (so we can skip those)
    sci_notation_pattern = re.compile(r'(?<![\w.])\d+\.?\d*[Ee][-+]?\d+')


    protected = {}
    def protect(match):
        key = f"__PROTECTED__{len(protected)}__"
        protected[key] = match.group(0)
        return key


    expr_protected = sci_notation_pattern.sub(protect, expr)


    replaced_count = 0
    for old, new in name_map.items():
        pattern = r'(?<![\w.])' + re.escape(old) + r'(?![\w])'
        new_expr, count = re.subn(pattern, new, expr_protected)
        if count > 0:
            logger.debug("Replaced '%s' with '%s' %d time(s)", old, new, count)
            expr_protected = new_expr
            replaced_count += count


    for key, val in protected.items():
        expr_protected = expr_protected.replace(key, val)


    if replaced_count > 0:
        logger.debug("Total replacements in expression: %d", replaced_count)


    return expr_protected


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


    main(args.path, **kwargs)
