#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
script name: petab_yamlpy
date: 11 -07 -2025
Author: Jonah R. Huggins

Description: shared-utility functions for Experiments class.

"""
# -----------------------Package Import & Defined Arguements-------------------#
import os, sys
from pathlib import Path

def make_default_root(default_path: os.PathLike | str, build_dir = "build") -> str:
    default_path = os.fspath(default_path)
    build_path = os.path.join(default_path, build_dir)

    os.makedirs(default_path, exist_ok=True)

    if not os.listdir(build_path):  # only extract if empty
        os.system(f"tar cf - . | (cd {default_path} && tar xvf -)")


    sys.path.append(default_path)


@staticmethod
def identifier_generator():
    """This function generates a unique identifier for the iterative
        of each simulation process.
    output:
        returns the unique identifier
    """
    import uuid

    identifier = str(uuid.uuid4())

    del uuid

    return identifier

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

