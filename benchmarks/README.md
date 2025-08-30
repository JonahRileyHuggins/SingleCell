# SPARCED Benchmarks

The SingleCell Experiment tool conducts paralellized simulation of PEtab-specified experiments, enabling efficient and reproducible, model-to-data comparisions. Each subdirectory here is a PEtab-Problem, defining a multitude of individual simulation settings in a tabular format that can be used to benchmark SPARCED model predictions as the model grows in scope.

## Individual Benchmark Overview

Benchmarks must be formatted according to [PEtab data format specifications](https://petab.readthedocs.io/en/latest/v1/documentation_data_format.html),  consistent with: 5 tab-separated value (tsv) files, an SBML file, and a yaml file (.yml) that specifies the path to each file.  We also include an additional section within the yaml file for defining SPARCED-specific attributes.

**Overview**

The Experiment tool recieves as input the path to a configuration YAML specifying paths to PEtab-Problem files. For further information on creating new benchmarks, please see the [PEtab data format specifications](https://petab.readthedocs.io/en/latest/v1/documentation_data_format.html).

**Example Configuration YAML**

```
parameter_file: ../LR-data/LR Model - Parameters.tsv
problems: 
  - name: 'test-benchmark'
    condition_files: 
    - conditions.tsv
    measurement_files:
    - measurements.tsv
    observable_files:
    - observables.tsv
    sbml_files:
    - ../LR_sbml_files/deterministic.xml
    - ../LR_sbml_files/stochastic.xml
    cell_count: 2

```

### Non-PEtab Attributes

Non-PEtab attributes `name` and `cell_count` represent SingleCell-specific details for simulations. The `name` attribute provides the framework with a name for results. The `cell_count` attribute is used by SingleCell for running multiple single-cell simulations for every conditions specified in the problems:condition_files input. SingleCell uses the Tau-Leaping algorithm to simulate stochastic gene expression, enabling stochastic-hybrid simulation.

#### Note On Scientific Notation in Observable Formulas

Given non-SingleCell species can have arbitrary names, we insist on using explict exponential notation for observable formulas rather than scientific notation. As one could imagine, its rather hard to separate the species 'E' from species 'Cd' if scientific notation is used (e.g. 'CdE-9' wouldn't be interpreted as "Cd times ten to the negative ninth power", but "Cd * E - 9").

## User Guide

To validate a single model benchmark (i.e. 'Stochastic Expression'), users pass the coresponding yaml file path to path flag for `SingleCell Experiment`. For simplicity, the names of each benchmark have been also used as the names for each yaml file. To operate, execute the following command (via Command Line Interface):

```
SingleCell Experiment --path [Path/To/benchmark.yaml]
```

Users can also iterate over every benchmark to validate all model predictions. To do so, simply execute the following command in the benchmarks/benchmark_utils/simulation/ directory:

```python
#ToDo: add method for multiple different experiments. Probably name-indexed Problems in config.yaml
```

## Small Kinetic Models with `SingleCell Experiment`

Support for alternative models is currently limited. Model details must be specified as tabular files input files and compiled into SBML and AMICI models using the following command:

```
SingleCell Build --path [path/to/Configuration YAML]

```
