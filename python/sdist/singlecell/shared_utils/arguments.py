#!/usr/bin/env python
# -*- coding: utf-8 -*-


# =========================================
# ============ Package Import ============
# =========================================
import os
import argparse
import pathlib

project_root = pathlib.Path(__file__).parent.parent.parent.parent.parent

ASCII_HEADER = r"""

  ░██████   ░██                      ░██              ░██████             ░██ ░██ 
 ░██   ░██                           ░██             ░██   ░██            ░██ ░██ 
░██         ░██░████████   ░████████ ░██  ░███████  ░██         ░███████  ░██ ░██ 
 ░████████  ░██░██    ░██ ░██    ░██ ░██ ░██    ░██ ░██        ░██    ░██ ░██ ░██ 
        ░██ ░██░██    ░██ ░██    ░██ ░██ ░█████████ ░██        ░█████████ ░██ ░██ 
 ░██   ░██  ░██░██    ░██ ░██   ░███ ░██ ░██         ░██   ░██ ░██        ░██ ░██ 
  ░██████   ░██░██    ░██  ░█████░██ ░██  ░███████    ░██████   ░███████  ░██ ░██ 
                                 ░██                                              
                           ░███████                                               
                                                                                  

   ╔═════════════════════════════════════════════════════════════╗
   ║  Software for building and running in-silico experiments    ║
   ║      with the Human Epithelial Cell Model                   ║
   ╚═════════════════════════════════════════════════════════════╝
"""

# =========================================
# ============ CLI Arguements ============
# =========================================
def parse_args():
      """Retrieve and parse arguments necessary for model creation

      Arguments:
            None

      Returns:
            A namespace populated with all the attributes.
      """
      # ========== [Parent Parser] =============
      parser = argparse.ArgumentParser(description=ASCII_HEADER,
          formatter_class=argparse.RawDescriptionHelpFormatter
      )

      # Parent-Shared arguments
      shared_parser = argparse.ArgumentParser(add_help=False)
      shared_parser.add_argument(
            '-v', '--verbose', 
            action='store_true', 
            help="Enable verbose output.", 
            dest="verbose"
      )
      shared_parser.add_argument(
            '-p', '--path', 
            help="General path, intended for configuration YAML file."
      )
      shared_parser.add_argument(
            '-n', '--name',
            help="String-type name"
      )
      shared_parser.add_argument(
            '--output', 
            '-o', 
            default = ".", 
            help  = "path to store output files"
      )
      shared_parser.add_argument(
            '-w', '--wild',
            help="UNDER CONSTRUCTION\nrunning wild (without SPARCED hard-coded values/behaviors"
      )

      # Subcommand definitions
      subparsers = parser.add_subparsers(dest="command", help="Commands: Build, Simulate, Experiment, Tool")


      # =========== [Command: Build] =============
      build_parser = subparsers.add_parser(
            "Build", 
            parents=[shared_parser],
            help="Build a model. SBML (and/or) AMICI"
      )
      build_parser.add_argument(
            '--catchall', 
            '-c', 
            metavar='KEY=VALUE', 
            nargs='*',
            help="Catch-all arguments passed as key=value pairs"
      )
      build_parser.add_argument(
            '--one4all', 
            default=True,
            help='Builds Deterministic SBML (and AMICI) model'
      )
      build_parser.add_argument(
            '--SBML_Only',
            help='List of solvers to skip AMICI compilation for (SBML-only build)',
            nargs='+',
            default=['stochastic']
      )
      build_parser.add_argument(
            '--build_dir', 
            help='CMake build directory for project C++ code',
            default=os.path.join(project_root, "build")
      )
      build_parser.add_argument(
            '--cmake_source_dir', 
            help='CMakeLists.txt directory for project C++ code',
            default=project_root
      )
      build_parser.add_argument(
            '--AMICI_Only', 
            help='Flag turns off SingleCell build command',
            action='store_false'
      )

      # =========== [Command: Simulate] =============
      # -- Lowercase
      simulate_parser = subparsers.add_parser(
            "Simulate", 
            parents=[shared_parser],
            help="Run a simulation."
      )
      simulate_parser.add_argument(
            '--sbml', 
            '-s', 
            help='SBMLs to be simulated.', 
            nargs='+', 
            default=['../sbml_files/Deterministic.sbml']
      )
      simulate_parser.add_argument(
            '--modify', 
            '-m', 
            metavar='KEY=VALUE', 
            nargs='+',
            help='Species to modify in key=value format',
            default=[]
      )
      simulate_parser.add_argument(
            '--start', 
            help = 'start time in seconds for simulation', 
            default = 0.0
      )
      simulate_parser.add_argument(
            '--stop', 
            help = 'stop time for simulation.', 
            default = 86400.0
      )
      simulate_parser.add_argument(
            '--step', 
            help = 'step size of each iteration in the primary for-loop.', 
            default = 30.0
      )


      # =========== [Command: Experiment] =============
      experiment_parser = subparsers.add_parser(
            "Experiment", 
            parents=[shared_parser],
            help="PEtab specified in-silico experiment."
      )
      # -- Lowercase
      experiment_parser.add_argument(
            '--cores', 
            '-c', 
            default=os.cpu_count(), 
            help = "Number of processes to divide tasks across"
      )
      experiment_parser.add_argument(
      '--No_Observables',
      action='store_false',
      help='Disable downsampling of data; specified in PEtab observables.tsv'
      )
      experiment_parser.add_argument(
            '--catchall', 
            metavar='KEY=VALUE', 
            nargs='*',
            help="Catch-all arguments passed as key=value pairs"
      )


      # ============ [Command: Tool] =================
      tool_parser = subparsers.add_parser(
            'Tool', 
            parents=[shared_parser], 
            help='Misc. helper tools for modeling'
      )
      tool_subparsers = tool_parser.add_subparsers(
            dest="tool_command", 
            help="Subcommands: unit_converter, inspector, species_name_converter"
            )
      
      # ---------- [Subcommand: unit converter] -------------
      uc_parser = tool_subparsers.add_parser(
            'unit_converter',
            help='Convert between nanomolar and molecules per cell'
      )
      uc_parser.add_argument(
          "--mpc", 
          help="target unit molecules / cell"
      )
      uc_parser.add_argument(
            "--nanomolar", 
            help = "target unit nanomolar"
      )
      uc_parser.add_argument(
            "-c", 
            "--compartment_volume", 
            help="volume (liters) of cellular compartment" \
                        "for unit conversion"
      )

      # ---------- [Subcommand: incorrect inspector] -------------
      ii_parser = tool_subparsers.add_parser(
            'incorrect_inspector',
            help='Find incorrectly specified parameters'
            )

      # ---------- [Subcommand: species name converter] ------------
      snc_parser = tool_subparsers.add_parser(
            'species_name_converter',
            help='Replaces old species names with new'
            )

      return(parser.parse_args())
