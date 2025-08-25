#!/bin/env python3 
# -*- coding: utf-8 -*-
"""
Author: JRH
Created: 05/15/2025
Description: Compiles the source code for SingleCell simulation code.
"""
import os
import subprocess
import pathlib
import logging

logging.basicConfig(
    level=logging.INFO, # Overriden if Verbose Arg. True
    format="%(asctime)s - %(levelname)s - %(message)s"
)
logger = logging.getLogger(__name__)

base_path = pathlib.Path(os.environ.get("SINGLECELL_PATH", pathlib.Path.home() / ".local/share/SingleCell"))

def build_singlecell(
        source_dir: os.PathLike | str = base_path,
        build_dir: os.PathLike | str = base_path / "build"
        ) -> None:
    """
    Compiles project source code

    :param source_dir (str):
        path to the source code directory

    : param build_dir (str):
        path to the build directory
    """
    logger.info('Compiling SingleCell source code')
    build_dir = pathlib.Path(build_dir)
    build_dir.mkdir(exist_ok=True)

    # Configure commands, mimics cmake build process
    subprocess.run(
        ["cmake", "-B", str(build_dir), "-S", str(source_dir)],
    check=True
    )

    # build commands, again mimicing cmake build process
    subprocess.run(
        ["cmake", "--build", str(build_dir), "--parallel"], 
        check=True
    )

    return # builds Cmake files, no returns
