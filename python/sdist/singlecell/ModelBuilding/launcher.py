# -*- coding: utf-8 -*-
#!/usr/bin/env python
"""
Created on Thurs. 05/16/2024 10:45:00 - JRH

Decider / Launcher script for Builder submodule. 

"""

# -----------------------Package Import & Defined Arguements-------------------#
import logging
from types import SimpleNamespace
from singlecell.ModelBuilding.build_process_organizer import Build_Organizer
logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)


# -----------------------Builder Class For Separating SBML and AMICI Builds-------------------------#
class Builder:

    def __init__(self, args: SimpleNamespace) -> None:
    
        builder = Build_Organizer(args=args)

        logger.info("Creating Antimony file(s)")
        builder.build_antimony_files()

        logger.info("Creating SBML model(s)")
        builder.build_sbml_models()

        logger.info("Compiling AMICI model(s)")
        builder.build_amici_models()

        logger.info("Compiling SingleCell code")
        builder.build_singlecell_code()

        
