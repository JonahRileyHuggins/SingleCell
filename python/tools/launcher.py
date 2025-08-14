# -*- coding: utf-8 -*-
#!/usr/bin/env python
"""
Created: 13/08/2025
Author: Jonah Huggins

Decider / Launcher script for tool belt submodule
"""
from incorrect_inspector import incorrect_inspector
from species_name_converter import convert
from unit_converter import nanomolar2mpc, mpc2nanomolar

from types import SimpleNamespace

class ToolBelt:

    def __init__(self, args: SimpleNamespace):
        if args.tool_command == 'unit_converter':
            if args.nanomolar:
                out = mpc2nanomolar(float(args.nanomolar), float(args.compartment_volume))
                print(out, " nM")
            elif args.mpc:
                out = nanomolar2mpc(float(args.mpc), float(args.compartment_volume))
                print(out, " molecules / cell")
        elif args.tool_command == 'incorrect_inspector':
            incorrect_inspector(args.path)

        elif args.tool_command == 'species_name_converter':
            convert(args.path, args.verbose)
