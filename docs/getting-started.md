# Running NS-Cache

NS-Cache is compiled as a standalone C++ executable. A run takes a configuration
file as its input and reports the selected memory organization and its circuit
characteristics. The commands below are executed from the repository root,
which is the reference directory for cell-file paths in the supplied examples.

## Compilation

Compilation requires Git, Make, and a C++ compiler supporting C++17. The Makefile invokes
`g++`; on macOS, the developer tools may provide this command through Clang.
Python is used for documentation generation and the separate regression suite.
The NS-Cache executable has no Python dependency.

```sh
git clone https://github.com/neurosim/NS-Cache.git
cd NS-Cache
make -C src -j2
```

The build produces `./nsc` in the repository root. Subsequent builds use
`make -C src -j2`. The compiler can be selected explicitly, for example with
`make -C src CXX=clang++ -j2`.

## SRAM cache example

```sh
./nsc config/New_Configs/SRAM_cache_14nm.cfg
```

This configuration specifies a 32 MiB, 16-way cache with a 512-bit (64-byte)
cache line, 14 nm LOP peripheral technology, H-tree routing, and read-latency
optimization. Its cell description is
`config/New_Configs/SRAM_cell_14nm.cell`.

The search evaluates the remaining activation patterns and multiplexing
choices within the specified organization constraints. The selected bank,
subarray, and mat dimensions are reported with their active counts. These
quantities determine the scope of the area and per-access energy estimates.

The console report can be saved through shell redirection:

```sh
./nsc config/New_Configs/SRAM_cache_14nm.cfg > sram-14nm.txt
```

Redirection replaces an existing file with the same name.

## Supplied configurations

All paths below are relative to the repository root.

| Configuration in `config/New_Configs/` | Modeled organization |
| --- | --- |
| `SRAM_cache_14nm.cfg` | Conventional FinFET SRAM cache |
| `SRAM_cache_10nm.cfg`, `SRAM_cache_7nm.cfg`, `SRAM_cache_5nm.cfg`, `SRAM_cache_3nm.cfg` | SRAM caches at the corresponding technology nodes |
| `M3D_SRAM_cache_14nm.cfg` | Monolithic mat folding enabled; broader organization search |
| `H3D_SRAM_cache_14nm.cfg` | Stacked-die example using the separate die/TSV controls |
| `eDRAM_Cache_14nm.cfg` | Conventional embedded-DRAM cache example |
| `GainCell_Cache_14nm.cfg` | Conventional gcDRAM cache using `config/Old_Configs/gcDRAM.cell` |
| `AOS_eDRAM_demo.cfg` | 8 KiB RAM with a fixed 512 x 128 data mat; device profile reconstructed from rounded legacy logs |
| `AOS_gcDRAM_demo.cfg` | 1 KiB RAM with a fixed 8 x 1024 data mat; synthetic read/write-device parameters |

The M3D and AOS configurations use the same invocation:

```sh
./nsc config/New_Configs/M3D_SRAM_cache_14nm.cfg
./nsc config/New_Configs/AOS_eDRAM_demo.cfg
./nsc config/New_Configs/AOS_gcDRAM_demo.cfg
```

The fixed AOS organizations reduce the number of candidates and the execution
time. Their device parameters are intended for model demonstration, with the
calibration basis described under [AOS devices](models/aos.md). The two examples
also use different CMOS peripheral nodes, which affect their relative latency,
energy, and area.

## Output organization

The report begins with the design specification, including capacity,
word width, enabled model options, and geometry constraints. The search then
reports `numSolutions` and `numDesigns`, corresponding to accepted and attempted
candidates, respectively. The selected organization specifies bank, subarray,
and mat dimensions, activation counts, mux factors, wire choices, and memory
tiers where applicable.

For a cache target, the results include a cache summary followed by data- and
tag-array breakdowns. A RAM target reports the memory array directly. The
[functional model](functionality.md) defines the reported timing, energy, and
refresh quantities. In particular, DRAM read access and full-cycle latency are
reported separately, and cache miss latency covers the cache-side lookup.

`No valid solutions.` indicates that the search produced no accepted
organization. The program may still print `Finished!` and return zero; the
solution count and exploration-failure messages therefore determine whether
the run produced a usable result.

## Configuration variants

A separate configuration file can be used for a parameter study:

```sh
cp config/New_Configs/AOS_eDRAM_demo.cfg my-edram.cfg
./nsc my-edram.cfg
```

The copied configuration retains its original `-MemoryCellInputFile` entry.
Device-level changes require a corresponding cell-file variant and an updated
path in the `.cfg` file. Capacity, word width, and forced dimensions jointly
constrain the candidate organization; incompatible values can eliminate the
design space. The [configuration parameters](configuration.md) describe these
dependencies.

## Execution diagnostics

| Reported condition | Relevant inputs |
| --- | --- |
| Configuration or cell file cannot be found | Working directory, configuration path, and `-MemoryCellInputFile` |
| No valid solutions | Capacity, word width, forced dimensions, mux factors, sensing mode, and bank aspect limit |
| Long exploration time | Number of free organization parameters and optimization mode; `Full` also writes candidate results |
| Input option has no effect | Key spelling and value case; unrecognized legacy keys can be ignored by the parser |
| AOS validation failure | Required device fields, units, temperature agreement, and on/off gate-voltage ordering |

Build rules are in [src/Makefile](https://github.com/neurosim/NS-Cache/blob/main/src/Makefile);
command-line and report flow are in
[src/main.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/main.cpp).
