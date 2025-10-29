# NeuroSim (NS)-Cache: An Early Exploration tool for FinFET and Nanosheet Generation Cache Memories

NeuroSim (NS)-Cache is a framework developed by [Prof. Shimeng Yu's group](https://shimeng.ece.gatech.edu/) (Georgia Institute of Technology) developed for early exploration of cache memories in advanced technology nodes (FinFET, nanosheet, CFET generations). The tool extends previously developed Destiny [2], NVSim [3], and Cacti3dd [4]. The leading-edge nodes transistor technology specifications and PPA at the circuit module are inherited from NeuroSim V1.4 [6]. The model is made publicly available on a non-commercial basis. Copyright of the model is maintained by the developers, and the model is distributed under the terms of the [Creative Commons Attribution-NonCommercial 4.0 International Public License](http://creativecommons.org/licenses/by-nc/4.0/legalcode)

If you use the tool or adapt the tool in your work or publication, please cite the following reference:

* F. Waqar, J. Kwak, J. Lee, M. Shon, O. Phadke, M. Gholamrezaei, K. Skadron, S. Yu, ※Optimization and Benchmarking of Monolithically Stackable Gain Cell Memory for Last-Level Cache, *§ IEEE Transactions Computers (T-Computer), 2025.*
<DOI to be generated>

:star2: This is the released version 1.0.0 (October, 2025) for the tool, and this version has **FinFET and Nanosheet generation technology integration, Peripheral circuit extensions, Gain-Cell parameterization and modifications for the Gem5 ecosystem**:. NS-Cache is currently a work-in-progress project in its initial iteration. Future releases will include packaged Gem5 [5] contents, improved documentation, tuned temperature variability models, and accessible technology extensions (i.e. amorphous oxide transistors).

```
The following is a list of the supported nodes with key features:

130nm Same as V1.3 (PTM Model)
90nm Same as V1.3 (PTM Model)
65nm Same as V1.3 (PTM Model)
45nm Same as V1.3 (PTM Model)
32nm Same as V1.3 (PTM Model)
22nm Same as V1.3 (PTM Model)
14nm FinFET, Fin number=4 (per each NMOS/PMOS)
10nm FinFET, Fin number=3 (per each NMOS/PMOS)
7nm FinFET, Fin number=2 (per each NMOS/PMOS)
5nm FinFET, Fin number=2 (per each NMOS/PMOS)
3nm FinFET, Fin number=2 (per each NMOS/PMOS)
2nm Nanosheet (GAA), Nanosheet number=3 (per each NMOS/PMOS), Backside power rail
1nm Nanosheet (GAA), Nanosheet number=4 (per each NMOS/PMOS), Backside power rail, Dielectric wall separation, CFET design for SRAM
```

## Getting Started
(1) To obtain the tool from GitHub and navigate to directory
```
git clone https://github.com/neurosim/NS-Cache.git
cd NS-Cache
```

(2) All source code is contained in `src/`. To build from home, use:
```
make -C src/
```

(3) Example configurations are contained in configs. To run, use:
```
./nsc <configuration_file>
// Example: ./nsc config/New_Configs/SRAM_cache_14nm.cfg
```

(4) To clean, use:
```
make clean -C src/
```

### Starter Tips
(1) Example configurations can be found in `config` in the home directory. The subdirectory `New_Configs` contains new cell definitions (`.cell`) and configurations (`.cfg`) for FinFET generation SRAM nodes, Heterogeneous 3D (H3D) and Monolithic 3D (M3D)* cache designs, and a Gain-Cell Design for those looking to get started. The subdirectory `Old_Configs` contains Destiny configurations (still compatible with NS-Cache). * The M3D flag does not replace the transistor technology. The user must specify what M3D memory properties are by adding cell/technology parameters.

(2) Looking to understand the set of available parameters? Check out `inputParameter.cpp` and `inputParameter.h` for configuration parameters. Check out `MemCell.cpp` and `MemCell.h` for cell definition parameters available in NS-Cache.

(3) Trying to add your own technology parameters? Try using `Technology.cpp` and `Technology.h` to add your own parasitics, and current density charecteristics.

(4) Co-Integrating with a cycle-based architectural simulator? Use the `-ViewQuantization` flag in your `.cfg` file in order to view quantized outputs for different levels of the hierarchy. Adjust the clock frequency/period using the `-ClockFrequency` flag with your specified frequency in Hz to change the quantization basis.

### Other
If you have logistic questions or comments, please contact :man: [Prof. Shimeng Yu](mailto:shimeng.yu@ece.gatech.edu), and if you have technical questions or comments, please contact :man: [Faaiq Waqar](mailto:faaiq.waqar@gatech.edu) or :man: [Junmo Lee](mailto:junmolee@gatech.edu) or :man: [Ming-Yen Lee](mailto:mlee838@gatech.edu).

This research is supported by the PRISM and CHIMES centers (SRC/DARPA JUMP 2.0).

### References
[1] F. Waqar, J. Kwak, J. Lee, M. Shon, O. Phadke, M. Gholamrezaei, K. Skadron, S. Yu, ※Optimization and Benchmarking of Monolithically Stackable Gain Cell Memory for Last-Level Cache, *§ IEEE Transactions Computers (T-Computer), 2025.

[2] M. Poremba, S. Mittal, D. Li, J.S Vetter, Y. Xie, ※Destiny: A tool for modeling emerging 3d nvm and edram caches, *§ IEEE Design, Automation & Test in Europe Conference & Exhibition (DATE) 2015

[3] X. Dong, C. Xu, Y. Xie, N. Jouppi, ※Nvsim: A circuit-level performance, energy, and area model for emerging nonvolatile memory, *§ IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems 2012

[4] K. Chen, S. Li, N. Muralimanohar, J.H Ahn, J.B Brockman, N. Jouppi, ※CACTI-3DD: Architecture-level modeling for 3D die-stacked DRAM main memory, *§ IEEE Design, Automation & Test in Europe Conference & Exhibition (DATE) 2012

[5] N. Binkert, B. Beckmann, G. Black, S.K Reinhardt, A. Saidi, J. Hestness, D.R Hower, T. Krishna, S. Sardashti, R. Sen ※The gem5 simulator, *§ ACM Sigarch computer architecture news 2011

[6] J. Lee, A. Lu, W. Li, S. Yu, ※NeuroSim V1. 4: Extending Technology Support for Digital Compute-in-Memory Toward 1nm Node, *§ IEEE Transactions on Circuits and Systems I: Regular Papers, 2024.
