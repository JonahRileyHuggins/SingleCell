#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse


def parse_args():
    """Retrieve and parse arguments necessary for model creation

    Arguments:
        None

    Returns:
        A namespace populated with all the attributes.
    """
    
    parser = argparse.ArgumentParser(prog="SPARCED", description="SPARCED CLI tool.")
    
    # Define shared arguments in a parent parser
    shared_parser = argparse.ArgumentParser(add_help=False)
    shared_parser.add_argument('-v', '--verbose', action='store_false', help="Enable verbose output.")
    shared_parser.add_argument('-y', '--yaml', help="YAML file with input configuration.")
    shared_parser.add_argument('-i', '--input_data',
                                 help="name of the subfolder containing SPARCED formatted input files")
    shared_parser.add_argument('-m', '--model',
                        help="relative path to the directory containing the \
                              models folders")
    shared_parser.add_argument('-n', '--name',
                        help="name of the model\nCompilation: desired name for \
                              the generated model (should be identical to \
                              model's folder name).\nSimulation: name of the \
                              input model.")
    shared_parser.add_argument('-w', '--wild',
                        help="UNDER CONSTRUCTION\nrunning wild (without SPARCED \
                              hard-coded values/behaviors")

    # Define subcommands
    subparsers = parser.add_subparsers(dest="command", help="Subcommands: compile, simulate, validate")

    # Compile subcommand
    compile_parser = subparsers.add_parser("compile", 
                                           parents=[shared_parser],
                                           help="Compile a model.")
    compile_parser.add_argument('-o', '--output_parameters',
                                 help="desired name for the output parameters file")

    # Simulate subcommand
    # -- Lowercase
    simulate_parser = subparsers.add_parser("simulate", 
                                            parents=[shared_parser],
                                            help="Run a simulation.")
    simulate_parser.add_argument('-p', '--population_size',
                                  help="desired cell population size for the simulation")
    simulate_parser.add_argument('-t', '--time',
                                  help="desired virtual duration of the simulation (h)")
    simulate_parser.add_argument('-r', '--results',
                                  help="directory where simulation results will be saved")
    simulate_parser.add_argument('-s', '--simulation',
                        help="desired name for the simulation output files")
    simulate_parser.add_argument('-x', '--exchange',
                         help="timeframe between modules information exchange \
                               during the simulation")
    # -- Uppercase
    simulate_parser.add_argument('-D', '--deterministic',
                        help="don't run simulation in deterministic mode")
    simulate_parser.add_argument('-P', '--perturbations',
                        help="name of the perturbations file to use (will \
                              override default)")

    # Benchmark subcommand
    benchmark_parser = subparsers.add_parser("validate", 
                                            parents=[shared_parser],
                                            help="Benchmark a model.")
    # -- Lowercase
    benchmark_parser.add_argument('-rs', '--return_sedml',default=False,
                        help="return the SED-ML file")
    benchmark_parser.add_argument('-r', '--results', default="./../results/New-Benchmark/",
                                   help="directory where benchmark results will be saved")
    benchmark_parser.add_argument('-b', '--benchmark', default=None,
                                  required=False,
                                  help="name of the benchmark to be used")
    benchmark_parser.add_argument('-c', '--cores',                default=1,
                        help="number of cores to use for a parallel process")
    benchmark_parser.add_argument('-bd', '--benchmark_description', default=None,
                        help="description of the benchmark")
    benchmark_parser.add_argument( '-a', '--run_all', help="run all benchmarks \
                                  in the benchmarks directory. This will override \
                                  the -b flag",
                                  required=False, default=None)
    # -- Uppercase
    benchmark_parser.add_argument('--Observable', default=1,
                        help="only the observable(s) in observables.tsv are calculated (1) \
                              or if the entire simulation is saved (0)")

    return(parser.parse_args())
