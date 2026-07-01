# Species Naming Conventions

This page defines the standardized naming conventions for intracellular species in [NAME], designed for compatibility with AMICI, PEtab, and SBML, and intent for:

1. **Uniqueness**: Each species name must be unique across the model
2. **Readability**: Names should be human-readable and concise, while avoiding ambiguity
3. **Compatibility**: Naming conventions comply with most systems biology software, including PEtab and SBML standards

Here, we define 'species' as entities which participate in reactions, located within a specific compartment. For further clarification see SBML Level 3 Version 2 ([here](https://sbml.org/specifications/sbml-level-3/version-2/core/release-2/sbml-level-3-version-2-release-2-core.pdf)). To reduce semantic ambiguity, we further define each section within the species name as a 'field' and provide short-form representations called 'tags'. There are currently 5 fields (of mixed requirement) within a species name: *Compartment, Type, State, Post-Translational Modifications* (*PTM*), and *Component*. Field definitions are provided in the **Current Field Tags** section. These rules are intended to change as we integrate new components; thus we welcome additions and ammendments!

## Rules

**Regex Compliance:** To enable query-able observables and consistent species nomenclature, we use the following rigid structure to describe individual species. Further information is provided in the **Regex Validation** section.

* Only letters, numbers, and underscores (`_`) are allowed

**Unified Structure**: All species names follow the format:

```
[Compartment][_Type_][State(Opt.)_][_PTM&Residue&Position(Opt.)_Component1_Isoform_][_PTM&Residue&Position(Opt.)_Component$n_Isoform_]
```

* Fields *Compartment*, *Type*, and *State* are applied to the entire species name, not individual components
* Fields *State* and *PTM* are conditionally optional. For further clarification on conditions, see individual field definitions
* Field order should follow the above specified naming structure of *Compartment*, *Type*, then *State* as the first prefixes of the species name. Individual components should be preceeded by *PTM, Residue,* and *Position*, then *component*
* Individual components are surrounded by single underscores (e.g. '`Component$n`'), thus any additional components will always be preceeded by two underscores (e.g. `_Component1__Component2_`)
* If multiple components have PTMs, place each individual PTM before the corresponding component
* Include isoform numbers of molecular species, when available. For further detail, see Isoform subsection below

![convention](./graphic.png)

See tables below for complete lists of currently represented items.

## Current Field Tags

### Compartment Field Tags

The shortened name of the compartment a particular species resides (e.g. `cyt_`, `nuc_`). A species compartment tag should match a **fieldId** entry within the compartments input file; if this is not the case, add a new entry to the compartment input file.

This table describes current compartment tags within the [NAME] model:


| Tag    | Description                                              |             Example             |
| ------ | -------------------------------------------------------- | :-----------------------------: |
| `exc`  | Species outside the cell, often noted as 'extracellular' |        `exc_prot__IFNG_1_`        |
| `cyt`  | Species in the cytoplasm                                 |        `cyt_prot__FOXO3_`        |
| `nuc`  | Species in the nucleus                                   |      `nuc_prot__pS332_ATM_`     |
| `mit`  | Species in the mitochondria                              |      `mit_prot__BAX__BCL2_`     |

### Type Field Tags

Type of species being labeled (e.g. `_prot`, `_lipid`). The following table specifies current species type tags:


|  Tag  | Precedence | Description                                                      |          Example          |
| :---------: | - | ------------------------------------------------------------------ | :--------------------------: |
|  `_abs_`  | 1 | "Abstract": Non-physical or non-explicitly represented entities  |    `cyt_abs__Ribosome_`    |
| `_cong_`  | 2 | "Conglomerate": Comprised of explicitly-defined, functionally redundant entities |     `nuc_cong__CCND_`     |
| `_mixed_` | 3 | "Mixed-type": Species with components of multiple types where none supercede in inheritance | `cyt_mixed__PIP3__PDK1_` |
| `_prot_` | 4 | "Protein": Represents a species that is comprised only of proteins |     `nuc_prot__DUSP1_`     |
| `_mrna_` | 4 | "mRNA": Represents an mRNA species                       |    `cyt_mrna__MYC_206_`    |
| `_gene_` | 4 | "Gene": Represents a gene species                        |    `nuc_gene_a__DUSP6_`    |
| `_comp_` | 4 | "Compound": Represents compound molecules, including drugs, small molecules, etc. | `cyt_comp__PIP3_` |

Note the *precedence* column insists on inheritance, where `_abs_` has top priority of inheritance, whereas `_mrna_` or `_prot_` are overriden if combined with any higher priority type. 
(e.g. `cyt_cong__AKT_ + cyt_comp__PIP3_ => cyt_cong__PIP3__AKT_`)


### State Field Tags [OPTIONAL]

Defines the status as applied to the entire species.

* **Condition**: If the species has no specified state, the state field may be excluded

The following table summarizes current states of species represented in the SPARCED model:


| Tag | Description      |             Example             |
| ---------- | ------------------ | :--------------------------------: |
| `in_`    | Internalized     | `cyt_prot_in__EGF__EGFR__ERBB2_` |
| `a_`     | Active state     |       `nuc_prot_a__TP53_`       |
| `i_`     | Inactive state |       `nuc_prot_i__CADH1_`       |

### Post Translational Modification Field Tags [OPTIONAL]

Prefixes indicating the post-translational modification (PTM) (e.g., `p` for phosphorylation, `u` for ubiquitination); PTMs are component-specific and should be placed before the individual component.

* **Residue&Position**: For species with specific post-translational modification (PTM) sites, denote the residue and its position (e.g., `_pS442`)
* **Condition**: If the species has no specified PTM, the PTM field may be excluded. Further, if the particular residue and/or position of a PTM is unknown / unable to be found in literature, default to the most specific PTM available (e.g. the position for `cyt_cong__u_CASP3and7_` is unkown, and literature suggests that this a polyubiquitination, therefore defaulting to `cyt_cong__u_CASP3and7_` is acceptable)

The following table summarizes current PTMs represented in the SPARCED model:


| Tag | Description     | Example                  |
| :--------- | ----------------- | -------------------------- |
| `_p`     | Phosphorylation | `nuc_prot__pS807_pS811_RB_` |
| `_u`     | Ubiquitination  | `cyt_cong__u_CASP3_` |
| `_cl`    | Cleavage         |      `cyt_prot__cl_PARP1_`      |



### Component Field Tags

Core molecule name. All components are separated by underscores on either side (e.g. '`_Component_Isoform_`'), the result is double underscores between components (e.g. `_JAK__INGR2_1_`). 

* Isoform Sub-Field: Molecules that have variants or isoforms need to be denoted as such and follow convention of the database where their respective annotation is assigned. This ensures no semantic ambiguity among variants

* Complexes :Each component is listed in order of annotation and follows their individual component rules for sourcing the name. Individual component PTMs should be placed before the corresponding component name.  Note that fields *Compartment*, *Type*, and *State* are only applied to the entire species name, not individual components
    * Example: `nuc_cong__CCND__CDK4_` represents Cyclin D and CDK4 forming a complex in the nucleus.

#### abstract (1)

Represents components with abstract representations, such as 'DNA damage', as well as biological entities without explicit representation. A good example of this is the species `cyt_abs__ribosome_`, which encapsulates the functional role of the ribosome without explicitly modeling its individual components. Ribosomes consist of ribosomal RNAs (rRNAs) and protein subunits that dynamically associate and disassociate during translation. By representing the ribosome implicitly, the model abstracts away the complexities of ribosome assembly, ribosomal subunit availability, and interactions with translation factors, instead focusing on its role as a resource for protein synthesis.
   * Annotations for abstract type species have thus far came from the Gene Ontology (GO)

#### conglomerates (2)

Conglomerates represent sets of functionally redundant species for a given component. Numeric distinctions (e.g., `Cyclin D1`, `Cyclin D2`) can be omitted for broader references like `CCND` when describing similar conglomerates, such as when the difference is not explicitly defined within the model.
   * To help differentiate between conglomerates containing multiple, similarly named species (e.g. proteins CDK4 and CDK6), we suggest using lower-case 'and' and 'or' in between such components within the species name (i.e. `nuc_cong__CDK4and6_`)
   * conglomerates lose specificity with PTM position

#### mixed (3)

Refers to species of two or more lower precedence types, such as the complex of a lipid with a protein, or a protein with a gene. 

#### Proteins (4)

Protein names should use the UniProt-assigned entry name, removing the '_HUMAN' portion, identifier when possible. 

* Annotation for **proteins** should use **Uniprot** identifiers. If numeric distinctions are omitted, include as annotation all referenced distinctions as intended by representation.

#### mRNA Transcripts (4)

mRNA transcripts adopt Enseml transcript names, with one variation. In order to maintain compliance with our Regex string,  we exchange the '-' character for an underscore (e.g., `TP53-201` becomes `TP53_201`). 

* Annotation for **mRNA** should come from **Ensembl** for the particular transcript name. If unknown, default to the Ensembl canonical transcript number.

#### Genes (4)

Where genes are included in the species list, they should use  HGNC-approved gene symbols (e.g. `CCND1`).

* Annotation for **genes**  should come from **Ensembl**, as the primary gene accession number.

#### Compounds (4)

Compounds should use the shortest Chemical Entities of Biological Interest (**ChEBI**) defined name or synonym that **does not** invalidate the species Regex (specified below). For example, **`PIP3`** would be a valid identifier for `1-phosphatidyl-1D-myo-inositol 3,4,5-trisphosphate`, however, **`PtsIns(3,4,5)P3`** invalidates the species Regex and is more verbose, thus should not be used. If all ChEBI names invalidate the Regex, remove all non-regex-compliant characters from the name, including spaces, dash, and commas (e.g. `PtsIns(3,4,5)P3` --> `PtsIns_3_4_5_P3`)

* Annotation for **compounds** should use **ChEBI** identifiers.
  
## Species Examples

##### Single Species with a State Field

- `mit_prot_a__BAX_`: active form of BAX protein, in the mitochondrion.

##### Two-Component Complex

- `cyt_cong__FGF__FGFR2_`: FGF bound to FGF Receptor 2 in the cytoplasm.

##### Mixed-Component Complex

* `cyt_cong__PIP3__p_p_AKT_`: cytoplasmic complex of PIP3 (compound) bound to the doubly phosphorylated protein AKT.

## Potential Pitfalls

1. **Ambiguity in Residue Position**:

   - Always specify residues and positions when possible for clarity in PTM names.
     - Example: Avoid `_p_CCND_` if `_pS15_CCND_` is more precise and known.

2. **Tool Compatibility**:

   - Avoid characters that invalidate the regex string. Again, only alphanumeric characters and underscores are valid species name characters.

## Regex Validation

The following regular expression ensures compatibility with PEtab, SBML, and PCRE2:
```text
^([a-z]{3}_)([a-z]{3,6}_)([a-zA-Z]{1,2}_)*(((_[a-z]{1}[A-Z]?[0-9]*)+)*((_([a-zA-Z0-9])+_)(([A-Z0-9]{1}[a-zA-Z0-9]*_){0,3})))+$
```

Below is a detailed implementation including comments for clarity:
```text
^ # Start
([a-z]{3}_)                # Compartment tag[Req]: exactly 3 lowercase letters & required _
([a-z]{3,6}_)              # Type Tag[Req]: 3–6 lowercase letters & required _
([a-zA-Z]{1,2}_)*          # State Tag[Opt]: 1–2 letters & required _
(                          # Start Component tag
   ( 
      (_[a-z]{1}[A-Z]?[0-9]*)+ # PTM tag[Opt]: require 1 lowercase letter & 0-1 uppr case letter(s) & numerics & required _
   )* # End PTM tag, Repeat 0+
   (
      (_([a-zA-Z0-9])+_) # Component Tag[Req]: required _ & 1+ alphanumeric(s) & required _
      (
         ([A-Z0-9]{1}[a-zA-Z0-9]*_){0,3} # Component carryover (includes isoform
      )
   )
)+ # End Component tag, Repeat 0+
$ # Finish

```

This PCRE2-compliant regex enforces:

- Alphanumeric names with underscores
- Double underscores (`__`) for complex species
- Compartment prefixes
