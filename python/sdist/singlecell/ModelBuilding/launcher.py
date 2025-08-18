# -*- coding: utf-8 -*-
#!/usr/bin/env python
"""
Created on Thurs. 05/16/2024 10:45:00 - JRH

Decider / Launcher script for Builder submodule. 

"""

# -----------------------Package Import & Defined Arguements-------------------#
import logging
from types import SimpleNamespace
from singlecell.ModelBuilding.sbml_model_builder import CreateModel
from singlecell.ModelBuilding.amici_model_builder import amici_builder, sanitize_multimodel_build
from singlecell.ModelBuilding.singlecell_builder import build_singlecell
from singlecell.shared_utils.utils import get_project_root
project_root = get_project_root()

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


# -----------------------Builder Class For Separating SBML and AMICI Builds-------------------------#
class Builder:

    def __init__(self, args: SimpleNamespace) -> None:
        logger.info("Creating SBML model(s)")
        stored_details = CreateModel(args= args)

        if args.SBML_Only:
            for sbml in args.SBML_Only:
                stored_details.sbml_paths.pop(sbml, None)
        
        for solver, path in stored_details.sbml_paths.items():
            logger.info("Compiling AMICI model '%s'", solver)

            amici_builder(sbml_path=path, model_name=solver, verbose=args.verbose)
        
        sanitize_multimodel_build()

        build_singlecell(args.cmake_source_dir, args.build_dir)
