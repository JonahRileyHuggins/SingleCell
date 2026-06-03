# User Guide

_Written by Jonah R. Huggins_

## Overview
⚠️ Work In Progress ⚠️ 

### Simple Operation
A container can be pulled from DockerHub that has the command-line interface pre-installed. The container is intended to be used from the command line, therefore running with an interactive (`-i`) teletypewriter (`-t`) flag is required. The below command will simultaneously pull the container from DockerHub and launch a session: 
```docker run -it --name demo jonahrileyhuggins/SingleCell:latest```

To restart the container from your last session, call the container by the assigned name, an example is provided below:
`docker start -ai demo`

#### SingleCell CLI Commands
The container has 3 primary commands, each with their own help profile. While the container should launch the command list on startup, to view the primary commands again, type
```SingleCell --help #or -h```

Further, to view the list of arguments and an example use case, you can use the `--help` flag with these as well
```SingleCell <Primary Command> --help```

### Command Line Interface
There are 3 basic functions present. It is required that the `Build` command be executed prior to other commands:
1. `SingleCell Build -h`
2. `SingleCell Simulate -h`
3.`SingleCell Experiment -h`

#### Global Options

These options are available for all commands.

| Flag | Description |
|--------|-------------|
| `-v`, `--verbose` | Enable verbose logging output. |
| `-p`, `--path PATH` | Path to input data file. |
| `-n`, `--name NAME` | Descriptive name for the run. |

---

#### Build

Construct SBML, AMICI, and/or SingleCell models.

```bash
SingleCell Build [options]
```

##### Build Options

| Flag | Description |
|--------|-------------|
| `-c`, `--catchall KEY=VALUE [KEY=VALUE ...]` | Additional build arguments supplied as key-value pairs. |
| `--one4all` | Build both deterministic SBML and AMICI models. |
| `--ANTIMONY_OUTPUT_DIR DIR` | Directory for generated Antimony files. |
| `--SBML_Only SOLVER [SOLVER ...]` | Skip AMICI/SingleCell compilation for the specified solver(s). Default: `stochastic`. |
| `--SBML_OUTPUT_DIR DIR` | Directory for generated SBML files. |
| `--DEFAULT_DETERMINISTIC_MODEL_PATH FILE` | SBML model used as input for AMICI compilation. |
| `--BUILD_AMICI_MODEL` | Build only the AMICI model from an existing SBML file. Skips Antimony, SBML, and SingleCell build stages. |
| `--AMICI_OUTPUT_DIR DIR` | Directory for generated AMICI models. |
| `--SINGLECELL_BUILD_DIR DIR` | CMake build directory used for SingleCell compilation. |
| `--SINGLECELL_CMAKE_SOURCE_DIR DIR` | Directory containing `CMakeLists.txt`. |
| `--COMPILE_SINGLECELL` | Compile SingleCell only. Skips Antimony, SBML, and AMICI build stages. |
| `--DISABLE_SINGLECELL_BUILD` | Disable SingleCell compilation after AMICI model generation. |

##### Examples

```bash
SingleCell Build
```

```bash
SingleCell Build --BUILD_AMICI_MODEL \
    --DEFAULT_DETERMINISTIC_MODEL_PATH deterministic.xml
```

```bash
SingleCell Build --COMPILE_SINGLECELL
```

---

#### Simulate

Run deterministic or stochastic simulations.

```bash
SingleCell Simulate [options]
```

##### Simulation Options

| Flag | Description |
|--------|-------------|
| `-s`, `--sbml FILE [FILE ...]` | One or more SBML models to simulate. |
| `-m`, `--modify KEY=VALUE [KEY=VALUE ...]` | Override initial species values. |
| `--start FLOAT` | Simulation start time (seconds). Default: `0`. |
| `--stop FLOAT` | Simulation stop time (seconds). Default: `86400` (24 hours). |
| `--step FLOAT` | Simulation step size (seconds). Default: `30`. |
| `-o`, `--output FILE` | Output TSV file. Defaults to a timestamped filename. |

##### Examples

```bash
SingleCell Simulate
```

```bash
SingleCell Simulate \
    --sbml deterministic.xml
```

```bash
SingleCell Simulate \
    --modify EGF=10 AKT=500
```

```bash
SingleCell Simulate \
    --start 0 \
    --stop 172800 \
    --step 60
```

---

#### Experiment

Execute PEtab-defined in-silico experiments.

```bash
SingleCell Experiment [options]
```

##### Experiment Options

| Flag | Description |
|--------|-------------|
| `-c`, `--cores N` | Number of worker processes. Default: system CPU count. |
| `--cache_dir DIR` | Directory used to store cached simulations. |
| `--load_index BOOL` | Load cached index from a previous experiment. |
| `--No_Observables` | Disable observable downsampling from `observables.tsv`. |
| `--catchall KEY=VALUE [KEY=VALUE ...]` | Additional experiment arguments. |
| `--run_all DIR` | Execute all benchmarks found in a directory. |

##### Examples

```bash
SingleCell Experiment
```

```bash
SingleCell Experiment --cores 32
```

```bash
SingleCell Experiment \
    --run_all benchmarks/
```

```bash
SingleCell Experiment \
    --cache_dir .cache \
    --load_index True
```

---

#### Tool

Miscellaneous helper utilities.

```bash
SingleCell Tool <subcommand> [options]
```

---

##### Tool: unit_converter

Convert between nanomolar concentration and molecules-per-cell.

```bash
SingleCell Tool unit_converter [options]
```

###### Options

| Flag | Description |
|--------|-------------|
| `--mpc VALUE` | Value in molecules per cell. |
| `--nanomolar VALUE` | Value in nanomolar. |
| `-c`, `--compartment_volume VOLUME` | Compartment volume in liters. |

###### Examples

```bash
SingleCell Tool unit_converter \
    --nanomolar 10 \
    --compartment_volume 1e-12
```

```bash
SingleCell Tool unit_converter \
    --mpc 50000 \
    --compartment_volume 1e-12
```

---

##### Tool: incorrect_inspector

Identify incorrectly specified parameters.

```bash
SingleCell Tool incorrect_inspector
```

No additional command-line arguments are currently defined.

---

##### Tool: species_name_converter

Convert deprecated species names to updated names.

```bash
SingleCell Tool species_name_converter
```

No additional command-line arguments are currently defined.

### Note! 
This is a work in progress, if you encounter errors during any process, following these steps helps in development:

1. Ensure you're working with the latest container version on DockerHub
2. If the error persists; navigate to the GitHub page and leave an issue describing the problem you've encountered
