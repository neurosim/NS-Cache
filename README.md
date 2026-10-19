# NS-Cache Student Lab

This branch contains the NS-Cache simulator and question-specific configurations for the memory-design course. Assignment instructions are provided separately by the instructor. Use the [output field guide](lab/FIELD_GUIDE.md) when reading simulation results.

## Setup

Use a macOS, Linux, or Windows WSL terminal with Git, Make, Bash, and a C++ compiler available as `g++`.

```bash
git clone --depth 1 --single-branch --branch student-lab https://github.com/neurosim/NS-Cache.git
cd NS-Cache
git rev-parse HEAD
make -C src -j2
./run q01 baseline
```

Record the full commit identifier printed above in your report and keep the revision assigned by your instructor, changing only the assigned configuration entries. Update only if instructed.

The baseline should produce a valid `RESULT` block. The run helper checks for both a positive `numSolutions` count and a result block.

## Edit and run

Each configuration is complete and ready to run. Edit only its marked block, using the settings assigned by the lab prompt. You may leave the fixed settings below that block unchanged. Configurations are given separately for each question, in order to limit confusion.

| Questions | Configuration | Editable lines | Example command |
| --- | --- | ---: | --- |
| Q1 bank baseline | [q01-bank.cfg](lab/configs/q01-bank.cfg) | 0 | `./run q01 baseline` |
| Q1–3 mat experiments | [q01-q03-mat.cfg](lab/configs/q01-q03-mat.cfg) | 3 mux factors | `./run q01-mat a` |
| Q4–5 partitioning | [q04-q05-partition.cfg](lab/configs/q04-q05-partition.cfg) | 1 bank-organization entry | `./run q04 case-a` |
| Q6–7 memory comparison | [q06-q07-memory.cfg](lab/configs/q06-q07-memory.cfg) | 1 cell-file entry | `./run q06 sram` |
| Q9 transfer width | [q09-transfer.cfg](lab/configs/q09-transfer.cfg) | 2 width/mux entries | `./run q09 128` |
| Q10–11 repeaters | [q10-q11-repeaters.cfg](lab/configs/q10-q11-repeaters.cfg) | 1 global-repeater entry | `./run q10 none` |

**The final argument labels the result. ** For example, `./run q02 d` uses the mat configuration as currently saved; it does not select case D. Edit the configuration first, then give the run a descriptive label.

Q3, Q5, Q7, and Q11 reuse earlier results. Q8 and Q12 require no additional simulation. The supplied cell definitions are [SRAM](lab/cells/SRAM_cell_14nm.cell) and [STT-MRAM](lab/cells/sample_STTRAM.cell); their parameters remain fixed for this lab.

Each run creates a unique folder under `results/` containing:

- `config.cfg`: the saved configuration used for that run;
- `raw.log`: complete simulator output, also displayed in the terminal;
- `revision.txt`: the commit identifier, working-tree status, and configuration ownership.

Keep these files with your calculations and plots. The helper runs from any working directory when invoked by its path; the examples above assume the repository root. Use `./run --help` for supported questions. 

```bash
./nsc lab/configs/q01-bank.cfg
```

## Model and attribution

NS-Cache is developed by Shimeng Yu's group at Georgia Institute of Technology and builds on DESTINY, NVSim, CACTI-3DD, and NeuroSim. These exercises use analytical model estimates; the course assignment identifies their assumptions and limitations. The [full project documentation](https://neurosimns-cache.readthedocs.io/en/latest/) provides additional background.

If you use or adapt the tool in work or a publication, cite F. Waqar, J. Kwak, J. Lee, M. Shon, O. Phadke, M. Gholamrezaei, K. Skadron, and S. Yu, “Optimization and Benchmarking of Monolithically Stackable Gain Cell Memory for Last-Level Cache,” *IEEE Transactions on Computers*, 2025. [DOI: 10.1109/TC.2025.3625490](https://doi.org/10.1109/TC.2025.3625490).

The model is distributed for non-commercial use under [CC BY-NC 4.0](LICENSE). Retain the source copyright notices and the inherited [DESTINY](LICENSE_DESTINY), [NVSim](LICENSE_NVSim), and [CACTI-3DD](LICENSE_CACTI3DD) license notices.
