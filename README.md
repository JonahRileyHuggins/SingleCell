# User Guide

_Written by Jonah R. Huggins_

## Overview
⚠️ Work In Progress ⚠️ 

Commands assume the SingleCell root directory (`/SingleCell` in the container). Use `SingleCell <command> --help` for full option lists.

### Quick Start
A container can be pulled from DockerHub that has the command-line interface pre-installed. The container is 
intended to be used from the command line, therefore running with an interactive (`-i`) teletypewriter (`-t`) flag 
is required. The below command will simultaneously pull the container from DockerHub and launch a session: 

Pull and start an interactive session:

```bash
docker run -it --name demo jonahrileyhuggins/singlecell:latest
```

To restart a named container:

```bash
docker start -ai demo
```

### CLI Commands
The container has 3 primary commands, each with their own help profile. While the container should launch the 
command list on startup, to view the primary commands again, type:
```bash
SingleCell --help #or -h
```

All examples below use a small test model in `tests/`. Run them from `/SingleCell` inside the container.

### Build
To create the various models (i.e. antimony, SBML, AMICI) and compile Linked- and simulatable-SingleCell code:

**Use:**
```bash
SingleCell Build --path <Path/to/configuration file>
```
This compiles model input files listed in the configuration file from tabular inputs into SBML (and AMICI/SingleCell binaries). 

**Example:**
```bash
SingleCell Build --path tests/LR-data/config.yaml --SBML_OUTPUT_DIR tests/LR_sbml_files -v
```

### Simulate
Run a single simulation using the built SBML models. Requires prior execution of **Build** step.

#### Fully Deterministic Simulations:
For simulations using deterministic-only methods; pass the path to the generated "One4All.xml" model:
```bash
SingleCell Simulate --sbml <path/to/One4All.xml> --output <output.tsv>
```
Note: This functionality is just using a wrapper around the compiled One4All AMICI model in `SingleCell/amici_models/`

#### Hybrid-Stochastic Simulations:
Alternatively to run hybrid-stochastic simulations; pass the paths to both the deterministic and stochastic SBML models:

**Use:**
```bash
SingleCell Simulate --sbml <path/to/deterministic.xml> <path/to/stochastic.xml> --output <output.tsv>
```
Note: The order in which you specify sbml paths **does not matter**.

**Example:**
```bash
SingleCell Simulate --sbml tests/LR_sbml_files/deterministic.xml tests/LR_sbml_files/stochastic.xml --modify cyt_prot__LIGAND_=18.5166 cyt_prot__RECEPTOR_=18.5166 --start 0 --stop 86400 --step 30 --output simulation_results.tsv
```

### Experiment
Run a simulation experiment formatted according to PEtab format (version 1) guidelines. Requires prior execution of **Build** step.

**Use:**
```bash
SingleCell Experiment --path <Path/to/benchmark.yaml>
```

**Example:**
```bash
SingleCell Experiment --path tests/LR-Benchmark/LR-benchmark.yaml -v
```

Note: Click Here for PEtab format guidelines (version 1)

## Note
This is a work in progress. If you encounter errors:

1. Ensure you're using the latest container from DockerHub.
2. If the error persists, open an issue on GitHub describing the problem.
