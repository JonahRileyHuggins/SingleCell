# Tab2SBML Input File Format Specification

## Overview

The Tab2SBML data format is a standardized framework for converting relational tables following best database management practices into SBML models, encoding biological models in a machine-readable format. It is designed to:

- Support model-merging methods for scalable model building
- Facilitate compatibility with PEtab, Antimony, and SBML

This format aims to create an easier entrypoint for scalable pathway model integration via model-merging by leveraging the benefits of both relational tables and SBML format. Relational tables are familiar entrypoints that support itemized data organization and are the primary user interface of many databases globally, whereas XML format (e.g. SBML) are static flat file representations that support easier data federation and heirarchical organization. This provides an easy structure for merging disparate pathway models into a single kinetic model. The model construction pipeline involves converting the tabular format to Antimony, then to SBML.

## General Principles

1. **Model-Merging**: Tabular files can be easily merged with widely available spreadsheet tools, enabling the combination of models from different sources.
2. **Compatibility**: Conversion from tabular format (inspired by PEtab) to Antimony and SBML
3. **Human-Readable**: Files are structured to be clear and easy to interpret
4. **Machine-Readable**: The format is well-defined for seamless software parsing

## File Organization

Tab2SBML files comprise 4 tabular files and a configuration (YAML) file, each representing critical data for systems biology models:

1. **Species**: Defines species, including names, initial concentrations, compartments, and UniProt annotations.
2. **RateLaws**: Specifies reaction details, rate laws, and references.
3. **Compartments**: Lists compartments, their sizes, and Gene Ontology (GO) annotations.
4. **Parameters**: Lists Rate constants for rate laws, defined in PEtab Parameter format. 
5. **YAML**: Organizes paths to various tabular files, incorporates misc. SBML-related details
**Note**: All model entities, column names, and row names are case-sensitive.


---

## Species Table

The `Species` table defines attributes of model species, including their properties and annotations.

| Column | **speciesId** | compartment | initialConcentration (nM) | solver | unit | **Annotation$n**  |
| ------ | ------------- | ----------- | -------------------- | ------ | -----|-----------------|
| Type   | STRING        | STRING      | FLOAT                | STRING | FLOAT | STRING         |
| Description | Unique identifier for the species. | Compartment where the species resides | Initial concentration of the species | SBML-compliant unit of entry | Identifiers (comma-separated, if multiple). |
| Example     | `cyt_cong__CCND_`                  | `Cytoplasm`                          | `0.787522147`      | Deterministic | nanoMolar | `P24385,P30279,P30281,P11802` |

### Field Descriptions

- **`speciesId`**: Follows the species naming conventions. Strict adherence ensures compatibility.
  - [More on naming conventions](https://github.com/JonahRileyHuggins/Human-Epithelial-Cell-Model/blob/main/docs/Species-Naming-Conventions.md)
- **`compartment`**: Must match a defined `Compartments` table entry.
- **`initialConcentration (nM)`**: Non-negative initial value for the entry in units nanoMolar. (Note, support for Molecule representation is onging)
- **`solver`**: Simulation Algorithm Identifier (currently only `Stochastic` and `Deterministic` available)
- **`unit`** (IGNORE FOR NOW): SBML-compliant unit of initial concentration
- **`Annotation`**: Relevant identifiers for the species, listed in alphanumeric order if multiple.

---

## RateLaws Table

The `RateLaws` table specifies reactions, their parameters, and associated rate laws. Rows align with columns in the `StoichiometricMatrix` input file.

| Column:      | reactionId                 | compartment                            | rateLaw                              | (Parameter 1: Leave Empty)   | ... | (Parameter N: Leave Empty) |
| ------------ | -------------------------- | -------------------------------------- | ------------------------------------ | ---------------------------- | --- | -------------------------- |
| Type:        | STRING                     | STRING                                 | STRING or FLOAT                      | FLOAT (OPTIONAL)             | ... | FLOAT (OPTIONAL)           |
| Description: | Unique reaction identifier | compartment where reaction takes place | Rate law formula or Constant         | First parameter for rate law | ... | Nth parameter for rate law |
| Example:     | vC23                       | Nucleus                                | kC23 * (Cd__Cdk4/()kC23_2+Cd__Cdk4)) | 0.09444444                   | ... | 10                         |

### Field Descriptions

- **`reactionId`**: Must be unique and follow naming conventions.
- **`compartment`**: Matches defined compartments in the `Compartments` table.
- **`rateLaw`**: Formula or mass-action constant with consistent species and parameter names.
- **`parameter 1`**: First parameter for the ratelaw. Do not include a label in the column header. If a constant was provided in rateLaw, leave blank. Parameters must be prefixed with `k` and are renamed for consistency during model generation.
- **`parameter N`**: Last parameter for the ratelaw. Do not include a label in the column header. If a constant was provided in rateLaw, leave blank. Parameters must be prefixed with `k` and are renamed for consistency during model generation.

### Notes for Users

- Ensure reaction compartments are defined in the `Compartments` table.
- Parameters are extracted and listed in the `ParamsAll` output file.
- The order of rows must match `StoichiometricMatrix` columns.

---

## Compartments Table

The `Compartments` table defines the spatial context for species and reactions.

| Column      | compartmentId                          | volume            | annotation                              | tagId  |
| ----------- | -------------------------------------- | ----------------- | --------------------------------------- |------  |
| Type        | STRING                                 | FLOAT             | STRING                                  | STRING |
| Description | Unique identifier for the compartment. | Volume in liters. | Gene Ontology term for the compartment. | Reference to Species Naming Convention guidelines on species compartment tag |
| Example     | `Cytoplasm`                          | `2.1e-12`       | GO:0005737                              | `cyt` |

### Field Descriptions

- **`compartmentId`**: Consistent with names in `Species` and `RateLaws`
- **`volume`**: Non-negative, defines compartment size for concentration scaling
- **`annotation`**: Gene Ontology identifier for specific compatment reference
- **`tagId`**: Three character prefix for reference in species naming convention

### Notes for Users

- Consistent compartment names across files are mandatory.
- Volumes impact concentration scaling for multi-compartment reactions.

---

## Parameter Table

The `Parameter` table defines attributes of model parameters, including scaling, bounds, and whether the parameter is estimated during optimization,strictly following the PEtab::Parameter table format.

| Column      | **parameterId**                      | parameterScale                                       | lowerBound                               | upperBound                               | nominalValue                        | estimate                                                      |
| ----------- | ------------------------------------ | ---------------------------------------------------- | ---------------------------------------- | ---------------------------------------- | ----------------------------------- | ------------------------------------------------------------- |
| Type        | STRING                               | STRING                                               | FLOAT                                    | FLOAT                                    | FLOAT                               | BOOL                                                          |
| Description | Unique identifier for the parameter. | Scaling applied to the parameter (`lin` or `log10`). | Lower bound of allowed parameter values. | Upper bound of allowed parameter values. | Default or nominal parameter value. | Indicates if the parameter is estimated (`1`) or fixed (`0`). |
| Example     | `kbR_1`                              | `lin`                                                | `0.0042`                                 | `0.0042`                                 | `0.0042`                            | `0`                                                           |

### Field Descriptions

* **`parameterId`**: Must be unique and follow naming conventions consistent with model references.
* **`parameterScale`**: Specifies the scale used for optimization (`lin`, `log10`, or `log`).
* **`lowerBound` / `upperBound`**: Define the parameter’s permissible range. Values outside this range are invalid.
* **`nominalValue`**: The baseline value used in simulations if estimation is disabled.
* **`estimate`**: Boolean flag; set to `1` if the parameter is subject to fitting, otherwise `0`.

---
## Configuration YAML

The configuration file defines all metadata and file paths required to compile a relational model specification into an SBML representation. It centralizes references to the core input tables and their locations, ensuring reproducible and portable model generation. This file serves as the primary control point for the conversion process.

### Example

```yaml (data/config.yaml)
# Genome Complete MCF10A Model

# General infos
name: "Human Epithelial Cell Model"
header: "Under Construction"
location: "."
version: "1.0"
description: "Configuration for loading reference and swappable input files"
author: "Jonah R. Huggins"

# Compilation
compilation:
    directory: "."
    files:
      compartments: "SPARCED - Compartments.tsv"
      ratelaws: "SPARCED - Ratelaws.tsv"
      species: "SPARCED - Species.tsv"
      parameters: "SPARCED - Parameters.tsv"
```

### Field Descriptions

* **`name`**: Descriptive name of the model.
* **`header`**: Optional label for logging.
* **`location`**: Root path for relative file references.
* **`version`**: User-defined model version tag.
* **`description`**: Short explanation of model purpose or composition.
* **`author`**: Creator or maintainer of the configuration.
* **`compilation`**: Defines the input files and directories required for model assembly.

  * **`directory`**: Path to the build or working directory.
  * **`files`**: Mapping of input table names to their corresponding TSV files.

This YAML file allows the converter to load the correct relational inputs and produce a consistent SBML model with minimal manual intervention.
