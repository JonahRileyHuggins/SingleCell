# SPARCED Benchmarking Pipeline

This tool serves as a means to rapidly and systematically compare SPARCED model predictions to experimental datasets. The curated directories listed here encompass 13 validated benchmarks that can be used to  inspect SPARCED model expansion for proper operation.

## Individual Benchmark Overview

Benchmarks must be formatted according to [PEtab data format specifications](https://petab.readthedocs.io/en/latest/v1/documentation_data_format.html),  consistent with: 5 tab-separated value (tsv) files, an SBML file, and a yaml file (.yml) that specifies the path to each file.  We also include an additional section within the yaml file for defining SPARCED-specific attributes.

**Overview**

The code framework uses paths specified in the yaml file to load in the corresponding PEtab files, as well as an instance of the SPARCED model at the SBML path, to simulate  individual conditions specified therin. If a user creates a new benchmark or instance of the SPARCED model, paths to the corresponding PEtab files (relative to the yaml file location) must be included within the yaml file.

**Example Yaml File**

```
format_version: 1 
parameter_file: ../../SPARCED/models/SPARCED_standard/data/model/output_parameters.txt
problems: 
  - condition_files: 
    - conditions.tsv
    measurement_files:
    - measurements.tsv
    observable_files:
    - observables.tsv
    sbml_files:
    - ../../SPARCED/models/SPARCED_standard/sbml_SPARCED_standard.xml
    model_specifications:
      hybrid: False
      cell_number: 1
      heterogenize_time: 30.0
```

### Creating The Model Specifications Section

The `model_specifications` section is SPARCED model-specific, enabling capabilities pertaining to the SPARCED model outside of PEtab's scope, such as the solver setting (deterministic or stochastic), multiple stochastic cells per simulation, and, starting stochastic cells with heterogeneous values from one another. A formatting guide is provided below to aid in creating new benchmarks:

#### Detailed field description

* `hybrid` [BOOL, OPTIONAL]
  Simulation setting for gene expression
* `cell_number` [NUMERIC, OPTIONAL]
  Number of cells to be simulated stochastically
* `heterogenize_time` [NUMERIC, OPTIONAL]
  Time length for simulated cells to be ran in a serum starved state to enable diversity among initial conditions between stochastic cells. in the time unit specified in the SBML model.

#### Note On Scientific Notation in Observable Formulas

Given that species can have arbitrary names, we insist on using explict exponential notation for observable formulas rather than scientific notation. As one could imagine, its rather hard to separate the species 'E' from species 'Cd' if scientific notation is used (e.g. 'CdE-9' wouldn't be interpreted as "Cd times ten to the negative ninth power", but "Cd * E - 9").

## User Guide

To validate a single model benchmark (i.e. 'Stochastic Expression'), users pass the coresponding yaml file path to the benchmarking operation script. For simplicity, the names of each benchmark have been also used as the names for each yaml file. To operate, execute the following command (via Command Line Interface):

```
mpiexec -n [CORES] sparced validate -b [Path/To/benchmark.yaml]
```

Users can also iterate over every benchmark to validate all model predictions. To do so, simply execute the following command in the benchmarks/benchmark_utils/simulation/ directory:

```
mpiexec -n [CORES] sparced validate -a [Path/To/benchmarks_dir]
```

## Small Kinetic Models with SPARCED

Any SBML model can be ran through this pipeline as long as it follows SPARCED compilation and simulation guidelines. Compiling a model is easy, given SPARCED input files have been created previously. For more information on file creation, see computational methods [here](https://www.nature.com/articles/s41467-022-31138-1). If SPARCED input files are created, navigate to the `SPARCED/SPARCED/models` and store said files. To compile a non-SPARCED kinetic model with SPARCED-input files, navigate to SPARCED/SPARCED/src and excute the `__main__.py` script:

```
sparced compile -n [model_name] -m [path/to/model_files]

```
