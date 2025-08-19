#!/usr/bin/env python
# -*- coding: utf-8 -*-


# =========================================
# ============ Package Import ============
# =========================================
import os
import argparse
import pathlib

project_root = os.path.join(pathlib.Path.home(), ".local", "share", "SingleCell")

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
            '--output', '-o', 
            default = ".", 
            help  = "path to store output files"
      )

      # Subcommand definitions
      subparsers = parser.add_subparsers(dest="command", help="Available commands")


      # =========== [Command: Build] =============
      build_parser = subparsers.add_parser(
            "Build", 
            parents=[shared_parser],
            help="Build a model. (SBML and/or AMICI)"
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
            nargs='+',
            default=['stochastic'],
            help ="Skip AMICI compilation for listed solvers (default: stochastic)."
      )
      build_parser.add_argument(
            '--build_dir', 
            default=os.path.join(project_root, "build"),
            help=f"CMake build directory for project C++ code (default: {os.path.join(project_root, 'build')})."
      )
      build_parser.add_argument(
            '--cmake_source_dir',
            default=project_root,
            help=f"Directory containing CMakeLists.txt (default: {project_root})."
      )
      build_parser.add_argument(
            '--AMICI_Only',
            action='store_false',
            help="Only build AMICI model (disable SingleCell build)."
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
            help="One or more SBML files to simulate (default: ../sbml_files/One4All.xml).", 
            nargs='+', 
            default=['../sbml_files/On4All.xml']
      )
      simulate_parser.add_argument(
            '--modify', '-m',
            metavar='KEY=VALUE',
            nargs='+',
            default=[],
            help="Initial species modifications in key=value format."
      )
      simulate_parser.add_argument(
            '--start',
            default=0.0,
            help="Simulation start time in seconds (default: 0.0)."
      )
      simulate_parser.add_argument(
            '--stop',
            default=86400.0,
            help="Simulation stop time in seconds (default: 86400.0 = 24 hrs)."
      )
      simulate_parser.add_argument(
            '--step',
            default=30.0,
            help="Simulation step size in seconds (default: 30.0)."
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
      action='store_true',
      help='Disable downsampling of data; specified in PEtab observables.tsv'
      )
      experiment_parser.add_argument(
            '--catchall', 
            metavar='KEY=VALUE', 
            nargs='*',
            help="Catch-all arguments passed as key=value pairs"
      )
      experiment_parser.add_argument(
            '--run_all', 
            help = 'Execute all benchmarks in a provided directory',
            default=None
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
