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
# ============ CLI Arguments ==============
# =========================================
def parse_args():
    """Retrieve and parse arguments necessary for model creation"""

    parser = argparse.ArgumentParser(
        description=ASCII_HEADER,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    # ---------- Shared / Global Options ----------
    shared_parser = argparse.ArgumentParser(add_help=False)
    global_group = shared_parser.add_argument_group("Global Options")
    global_group.add_argument(
        '-v', '--verbose',
        action='store_true',
        dest="verbose",
        help="Enable verbose logging output."
    )
    global_group.add_argument(
        '-p', '--path',
        help="Path to data file."
    )
    global_group.add_argument(
        '-n', '--name',
        help="Descriptive name for this run."
    )
    global_group.add_argument(
        '-o', '--output',
        default=".",
        help="Directory to store output files (default: current directory)."
    )

    # ---------- Subcommands ----------
    subparsers = parser.add_subparsers(dest="command", help="Available commands")

    # =========== [Command: Build] ============
    build_parser = subparsers.add_parser(
        "Build",
        parents=[shared_parser],
        help="Construct models (SBML and/or AMICI)."
    )
    build_group = build_parser.add_argument_group("Build Options")
    build_group.add_argument(
        '--catchall', '-c',
        metavar='KEY=VALUE',
        nargs='*',
        help="Additional build arguments in key=value format."
    )
    build_group.add_argument(
        '--one4all',
        default=True,
        help="Build both deterministic SBML and AMICI model (default: True)."
    )
    build_group.add_argument(
        '--ANTIMONY_OUTPUT_DIR',
        default=os.path.join(project_root, "sbml_files"),
        help=f"Antimony output directory (default: {os.path.join(project_root, 'sbml_files')})."
    )
    build_group.add_argument(
        '--SBML_Only',
        nargs='+',
        default=['stochastic'],
        help="Skip AMICI compilation for listed solvers (default: stochastic)."
    )
    build_group.add_argument(
        '--SBML_OUTPUT_DIR',
        default=os.path.join(project_root, "sbml_files"),
        help=f"SBML output directory (default: {os.path.join(project_root, 'sbml_files')})."
    )
    build_group.add_argument(
        '--AMICI_Only',
        action='store_false',
        help="Only build AMICI model (disable SingleCell build)."
    )
    build_group.add_argument(
        '--AMICI_OUTPUT_DIR',
        default=os.path.join(project_root, "amici_models"),
        help=f"AMICI output directory (default: {os.path.join(project_root, 'amici_models')})."
    )
    build_group.add_argument(
        '--SINGLECELL_BUILD_DIR',
        default=os.path.join(project_root, "build"),
        help=f"CMake build directory (default: {os.path.join(project_root, 'build')})."
    )
    build_group.add_argument(
        '--SINGLECELL_CMAKE_SOURCE_DIR',
        default=project_root,
        help=f"Directory containing CMakeLists.txt (default: {project_root})."
    )

    # =========== [Command: Simulate] =========
    simulate_parser = subparsers.add_parser(
        "Simulate",
        parents=[shared_parser],
        help="Run deterministic/stochastic simulations."
    )
    sim_group = simulate_parser.add_argument_group("Simulation Options")
    sim_group.add_argument(
        '--sbml', '-s',
        nargs='+',
        default=['../sbml_files/One4All.xml'],
        help="One or more SBML files to simulate (default: ../sbml_files/One4All.xml)."
    )
    sim_group.add_argument(
        '--modify', '-m',
        metavar='KEY=VALUE',
        nargs='+',
        default=[],
        help="Initial species modifications in key=value format."
    )
    sim_group.add_argument(
        '--start',
        type=float,
        default=0.0,
        help="Simulation start time in seconds (default: 0.0)."
    )
    sim_group.add_argument(
        '--stop',
        type=float,
        default=86400.0,
        help="Simulation stop time in seconds (default: 86400.0 = 24 hrs)."
    )
    sim_group.add_argument(
        '--step',
        type=float,
        default=30.0,
        help="Simulation step size in seconds (default: 30.0)."
    )

    # =========== [Command: Experiment] ======
    experiment_parser = subparsers.add_parser(
        "Experiment",
        parents=[shared_parser],
        help="Execute PEtab-specified in-silico experiments."
    )
    exp_group = experiment_parser.add_argument_group("Experiment Options")
    exp_group.add_argument(
        '--cores', '-c',
        type=int,
        default=os.cpu_count(),
        help=f"Number of parallel processes (default: {os.cpu_count()})."
    )
    exp_group.add_argument(
        '--No_Observables',
        action='store_true',
        help="Disable observable downsampling defined in observables.tsv."
    )
    exp_group.add_argument(
        '--catchall',
        metavar='KEY=VALUE',
        nargs='*',
        help="Additional experiment arguments in key=value format."
    )
    exp_group.add_argument(
        '--run_all',
        default=None,
        help="Execute all benchmarks in a given directory."
    )

    # ============ [Command: Tool] ===========
    tool_parser = subparsers.add_parser(
        'Tool',
        parents=[shared_parser],
        help='Miscellaneous helper tools for modeling.'
    )
    tool_subparsers = tool_parser.add_subparsers(
        dest="tool_command",
        help="Available tool subcommands"
    )

    # ----- [Tool: unit converter] -----------
    uc_parser = tool_subparsers.add_parser(
        'unit_converter',
        help='Convert between nanomolar and molecules per cell.'
    )
    uc_group = uc_parser.add_argument_group("Unit Converter Options")
    uc_group.add_argument(
        "--mpc",
        help="Value in molecules per cell."
    )
    uc_group.add_argument(
        "--nanomolar",
        help="Value in nanomolar."
    )
    uc_group.add_argument(
        "-c", "--compartment_volume",
        help="Compartment volume in liters (used for conversion)."
    )

    # ----- [Tool: incorrect inspector] -----
    tool_subparsers.add_parser(
        'incorrect_inspector',
        help='Identify incorrectly specified parameters.'
    )

    # ----- [Tool: species name converter] ---
    tool_subparsers.add_parser(
        'species_name_converter',
        help='Replace old species names with new names.'
    )

    return parser.parse_args()
