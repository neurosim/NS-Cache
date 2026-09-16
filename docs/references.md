# References and model provenance

Reference numbers follow the
[repository README](https://github.com/neurosim/NS-Cache#references).
Journal citations use final issue dates.

## NS-Cache research paper

[1] F. G. Waqar et al., *Optimization and Benchmarking of Monolithically
Stackable Gain Cell Memory for Last-Level Cache*, IEEE Transactions on
Computers, vol. 75, no. 3, pp. 760–775, March 2026.
[DOI: 10.1109/TC.2025.3625490](https://doi.org/10.1109/TC.2025.3625490).

Primary citation for NS-Cache; describes M3D gain-cell cache modeling and
gem5-based benchmarking. [Author preprint](https://arxiv.org/abs/2503.06304).

Figures 3, 4, and 6 are reproduced in these pages with their original numbering.
© 2025 IEEE; [publisher reuse terms](https://www.ieee.org/publications/rights/index.html)
apply to the reproduced images.

## Inherited modeling tools

### DESTINY

[2] M. Poremba et al., *DESTINY: A Tool for
Modeling Emerging 3D NVM and eDRAM Caches*, Design, Automation and Test in
Europe (DATE), pp. 1543–1546, 2015.
[DOI: 10.7873/DATE.2015.0733](https://doi.org/10.7873/DATE.2015.0733).

Provides the inherited memory-organization search and hierarchical circuit
evaluation. [Original repository](https://code.ornl.gov/3d_cache_modeling_tool/destiny).

The companion *A Manual on use of DESTINY Tool for Architectural Studies*,
updated May 3, 2015, describes architectural modeling, configuration inputs,
and hierarchical results in Sections I–II.

### NVSim

[3] X. Dong, C. Xu, Y. Xie, and N. P. Jouppi, *NVSim: A Circuit-Level
Performance, Energy, and Area Model for Emerging Nonvolatile Memory*, IEEE
Transactions on Computer-Aided Design of Integrated Circuits and Systems,
vol. 31, no. 7, pp. 994–1007, 2012.
[DOI: 10.1109/TCAD.2012.2185930](https://doi.org/10.1109/TCAD.2012.2185930).

Provides the nonvolatile-memory circuit-model foundation inherited through
DESTINY, including latency, energy, and area estimation.

### CACTI-3DD

[4] K. Chen et al., *CACTI-3DD: Architecture-Level Modeling for 3D Die-Stacked DRAM
Main Memory*, Design, Automation and Test in Europe (DATE), pp. 33–38, 2012.
[DOI: 10.1109/DATE.2012.6176428](https://doi.org/10.1109/DATE.2012.6176428).

Provides the inherited die-stacked DRAM and vertical-interconnect modeling
background, distinct from NS-Cache's [mat-level M3D model](models/monolithic-3d.md).

### NeuroSim 1.4

[6] J. Lee, A. Lu, W. Li, and S. Yu, *NeuroSim V1.4: Extending Technology Support
for Digital Compute-in-Memory Toward 1nm Node*, IEEE Transactions on Circuits
and Systems I: Regular Papers, vol. 71, no. 4, pp. 1733–1744, April 2024.
[DOI: 10.1109/TCSI.2024.3362822](https://doi.org/10.1109/TCSI.2024.3362822).

Supplies the [advanced CMOS technology models](models/transistor-technology.md).
Section II covers geometry, devices, and interconnect; Appendix Tables III–IV
list projected device parameters.

## Architectural simulation

[5] N. Binkert et al., *The gem5 Simulator*, ACM SIGARCH Computer Architecture
News, vol. 39, no. 2, pp. 1–7, 2011.
[DOI: 10.1145/2024716.2024718](https://doi.org/10.1145/2024716.2024718).

Provides workload-level architectural simulation using cache parameters
derived from NS-Cache. [gem5 publications](https://www.gem5.org/publications/).

## SRAM macro comparisons

These papers underpin the [README validation table](https://github.com/neurosim/NS-Cache#validation).
The comparisons have not been reproduced for this documentation revision.

| Node | Reference | Capacity label | Compared quantities |
| --- | --- | --- | --- |
| 7 nm | [7] M. Clinton et al., *A 5GHz 7nm L1 Cache Memory Compiler for High-Speed Computing and Mobile Applications*, IEEE International Solid-State Circuits Conference (ISSCC), pp. 200–201, 2018. [DOI: 10.1109/ISSCC.2018.8310253](https://doi.org/10.1109/ISSCC.2018.8310253). | 74 kb | Access time; area |
| 5 nm | [8] T.-Y. J. Chang et al., *A 5-nm 135-Mb SRAM in EUV and High-Mobility Channel FinFET Technology With Metal Coupling and Charge-Sharing Write-Assist Circuitry Schemes for High-Density and Low-VMIN Applications*, IEEE Journal of Solid-State Circuits, vol. 56, no. 1, pp. 179–187, January 2021. [DOI: 10.1109/JSSC.2020.3034241](https://doi.org/10.1109/JSSC.2020.3034241). | 135 Mb/144 kb | Read access time; area |
| 3 nm | [9] Y. Osada et al., *3.7-GHz Multi-Bank High-Current Single-Port Cache SRAM With Leakage Saving Circuits in 3-nm FinFET for HPC Applications*, IEEE Journal of Solid-State Circuits, vol. 60, no. 3, pp. 1113–1121, March 2025. [DOI: 10.1109/JSSC.2024.3440970](https://doi.org/10.1109/JSSC.2024.3440970). | 360 kb | Access time; area; leakage; read/write energy |

## NS-Cache

The [NS-Cache source](https://github.com/neurosim/NS-Cache), revision
[`d6e2000`](https://github.com/neurosim/NS-Cache/tree/d6e2000), defines the
configuration syntax, model equations, and result accounting described here.
The following links refer to the maintained `main` branch.

| Source | Responsibility |
| --- | --- |
| [main.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/main.cpp) | Program entry, technology setup, candidate search, and reporting |
| [InputParameter.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/InputParameter.cpp) | `.cfg` parsing and top-level defaults |
| [MemCell.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/MemCell.cpp) | `.cell` parsing, AOS validation, and retention handling |
| [Technology.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Technology.cpp) and [formula.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/formula.cpp) | CMOS device tables and geometry/electrical equations |
| [Mat.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Mat.cpp) | Cell-array circuits, DRAM/gcDRAM behavior, and M3D mat accounting |
| [AOSFETCompactModel.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/AOSFETCompactModel.cpp) | Oxide-device compact equations and operating points |
| [Result.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Result.cpp) | Report definitions, component accounting, and cache summaries |
| [config/New_Configs](https://github.com/neurosim/NS-Cache/tree/main/config/New_Configs) | Current example configuration/cell pairs |

The AOS eDRAM example reconstructs part of a device profile from rounded legacy
execution logs; the original calibrated cell file was unavailable. The AOS
gcDRAM example uses synthetic parameters. The [AOS model description](models/aos.md)
records the resulting calibration limits.

Research citation and licensing information for NS-Cache itself are maintained
in the [repository README](https://github.com/neurosim/NS-Cache#readme) and
[LICENSE](https://github.com/neurosim/NS-Cache/blob/main/LICENSE).

## Acknowledgments

Developed by Prof. Shimeng Yu's group at Georgia Tech, with support acknowledged
from PRISM and CHIMES under SRC/DARPA JUMP 2.0.
