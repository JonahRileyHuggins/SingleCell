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
def build_singlecell(
        source_dir: os.PathLike | str = '../../../../',
        build_dir: os.PathLike | str = '../../../../build'
        ) -> None:
    """
    Compiles project source code

    :param source_dir (str):
        path to the source code directory

    : param build_dir (str):
        path to the build directory
    """

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


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(prog='SingleCell_Builder')
    parser.add_argument(
        '--build_dir', 
        help='CMake build directory for project C++ code',
        default=os.path.join("../../../../build")
    )
    parser.add_argument(
        '--cmake_source_dir', 
        help='CMakeLists.txt directory for project C++ code',
        default='../../../../'
    )
    args = parser.parse_args()
    build_singlecell(args.cmake_source_dir, args.build_dir)