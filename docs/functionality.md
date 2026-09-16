# Functionality and results

NS-Cache evaluates the area, latency, dynamic energy, and leakage of candidate memory organizations using analytical circuit models. The calculation spans the device, peripheral circuit, and bank hierarchy described in [Architecture](architecture.md). Workload behavior, including cache contents, replacement decisions, coherence, and request scheduling, is outside this calculation. The resulting circuit metrics can be used as inputs to an architectural simulator, following the usage described in Section I of the [DESTINY manual](references.md#destiny).

## From configuration to candidate result

The evaluation begins with the top-level [configuration](configuration.md), technology initialization, and cell-file validation. For a cache target, the tag capacity is derived first and candidate tag-array organizations are evaluated. The data-array search then incorporates the selected cache access mode. A RAM target evaluates the data array without the tag search. The execution path is implemented in [main.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/main.cpp), with search loops and evaluation helpers in [macros.h](https://github.com/neurosim/NS-Cache/blob/main/src/macros.h).

Candidate organizations are formed from the permitted array dimensions, activation counts, multiplexing factors, buffer choices, and stacking options. Mat rows and columns are derived from capacity and addressing constraints. A forced mat size retains only candidates whose derived dimensions match the requested dimensions.

Each candidate is initialized and evaluated for area, resistance and capacitance, latency, and power. The calculations propagate from the local mat circuitry through the subarray and bank routing. Valid candidates are compared against the selected optimization objective and metric constraints. Local and global wire choices are then refined around retained results. Full exploration can also produce CSV records for the evaluated designs.

The selected result is the optimum within the implemented candidate space and modeling assumptions. An empty result indicates that no evaluated organization satisfies all validity conditions and constraints; possible causes include incompatible array dimensions, unsupported sensing choices, or unsatisfied refresh requirements.

## What each operation models

A read includes address decoding, wordline activation, cell and bitline response, sensing, output selection, and bank routing. Parallel circuit paths contribute through their maximum delay, while successive stages contribute through the sum of their delays. The read latency therefore depends on the path structure in addition to the individual component delays. Precharge establishes the initial bitline condition and contributes to the access or cycle latency according to the memory type.

A write includes driving the selected path and performing the cell-specific storage operation. Nonvolatile memory models may include separate set and reset pulse contributions. In gcDRAM, the read and write paths have separate access-device and bitline properties, and the write-driver energy contributes once to the write total. Cell operations and local peripheral contributions are implemented in [Mat.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Mat.cpp).

For DRAM and eDRAM, the read access latency represents the time required to obtain the data. Restoration follows the access and completes the read cycle:

```text
mat full read-cycle latency = mat access latency + mat restore delay
```

The bank `Read Latency` includes the response path through the surrounding hierarchy and excludes the mat's post-read restoration. Read bandwidth is calculated using the full mat cycle. The `DRAMTargetResidualRatio` parameter specifies the settling accuracy for write-bitline and restoration delays; reducing the residual ratio increases the required settling time. Refresh includes restoration for DRAM/eDRAM and the modeled read/write sequence for gcDRAM.

Refresh energy aggregates the operations required by the refresh model. The detailed output reports the per-row contributions used to reconstruct this energy. Higher-level aggregation and reporting are implemented in [SubArray.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/SubArray.cpp) and [Result.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Result.cpp).

## Interpreting the report

| Metric | Definition |
| --- | --- |
| Area | Footprint of the specified hierarchy and its peripherals. Cache summary area is the sum of data and tag areas. M3D projected footprint, tier areas, and MIV area are reported separately. |
| Read/write latency | Operation response time at the named hierarchy level. Internal values are seconds; reports choose displayed units. |
| Dynamic energy | Energy of the specified operation, in joules internally. Component labels specify the aggregation level, such as per active mat, per row, or per bank. |
| Leakage power | Background power estimate in watts. DRAM-family paths retain a refresh-related contribution in this field, as described below. |
| Refresh power | Reported bank refresh dynamic energy divided by retention time. |
| Read/write bandwidth | Analytical throughput estimate from block size and the relevant mat cycle. It does not include workload contention or a memory-controller schedule. |
| Energy-delay product | Read or write dynamic energy multiplied by its corresponding response latency. DRAM read EDP uses time-to-data rather than restoration-inclusive cycle time. |

Indented circuit contributions form a decomposition of their parent total. For DRAM, eDRAM, and gcDRAM, the `leakage` field includes both peripheral leakage and an inherited refresh-energy proxy based on `DRAM_REFRESH_PERIOD`. Adding the separately reported refresh power directly to this field can therefore count refresh-related power more than once. In AOS configurations, modeled leakage also includes the full-`Vds` leakage upper bound described in [AOS options](models/aos.md). This contribution is an uncalibrated upper bound on the device leakage under the specified bias assumptions. The M3D area decomposition is described in [Monolithic 3D](models/monolithic-3d.md).

At cache-summary level, `Cache Refresh Power` is calculated from the data-bank refresh energy, while `Cache Refresh Dynamic Energy` includes both data and tag contributions. These fields consequently have different aggregation scopes.

## Cache access modes and external simulation

Cache metrics combine the data- and tag-array results according to the access mode. Sequential mode adds the tag and data read latencies. Fast mode overlaps the complete tag and data reads. Normal mode overlaps tag lookup with data-row activation, followed by column selection and the remaining routing. `Cache Miss Latency` represents tag-lookup completion; fetching the requested data from a lower memory level is outside this latency.

For writes, all three access modes use the maximum of the tag and data write latencies. The sequential-mode write expression therefore differs from the equation in the historical DESTINY manual. Normal- and fast-mode miss energy also includes an approximate data-access contribution. These expressions are implemented in [Result.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Result.cpp).

The clock-derived cycle counts and gem5-oriented parameters convert the analytical estimates into quantities suitable for subsequent architectural simulation. NS-Cache does not execute gem5 during this conversion. System-level energy and performance estimates additionally require workload access counts, elapsed time, miss penalties, and scheduling assumptions. Reproducibility depends on the source revision, top-level configurations, cell files, and model-version labels associated with each result.
