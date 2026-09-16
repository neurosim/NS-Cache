# Configuration parameters

The model inputs are divided into system-level specifications and cell-level
parameters. The `.cfg` file defines capacity, organization constraints,
peripheral technology, and the optimization objective. The `.cell` file
defines cell geometry, access-device properties, and the electrical parameters
of the storage element. Each `-MemoryCellInputFile` entry selects a cell
definition; multiple entries enable a sweep across cell types.

## File syntax and units

An option occupies one line, starting at the first character:

```text
-Capacity (KB): 64
-WordWidth (bit): 512
-MemoryCellInputFile: config/New_Configs/SRAM_cell_14nm.cell
```

Option names include their parenthesized units. Older enumerated values are
case-sensitive, while selected newer controls use strict parsing as specified
below. Whole-line comments begin with `#` or `//`. Strictly parsed options
reject trailing text, including inline comments. The inherited filename parser
accepts a single whitespace-delimited token, so paths cannot contain spaces.

Capacity is in bytes: `KB` and `MB` use factors of 1024. Word width is in bits;
for a cache it is the cache-line width. Temperatures are in kelvin.

In geometry fields, `F` represents the configured technology-node size:
`F = ProcessNode × 10⁻⁹ m`, stored as `Technology::featureSize`. Thus, at
`-ProcessNode: 7`, `1 F = 7 nm` and `1 F² = 49 nm²`. Cell areas marked `(F^2)`
and transistor widths marked `(F)` use this normalization. At advanced nodes,
`F` is a scaling unit, not the physical gate length, fin pitch, or nanosheet
width; those dimensions are separate technology parameters.

For capacitance fields such as `-DRAMCellCapacitance (F)` and AOS
`OverlapCapacitance (F)`, `F` instead denotes farads. AOS widths and lengths
marked `(m)` are explicit SI dimensions, independent of the `.cfg` process-node
label.

## System and technology specifications

| Option | Definition |
| --- | --- |
| `-DesignTarget` | `cache` evaluates data and tag arrays; `RAM` evaluates a memory array. These values are case-sensitive. |
| `-Capacity (B)`, `-Capacity (KB)`, `-Capacity (MB)` | Alternative unit forms for the requested data capacity. |
| `-WordWidth (bit)` | I/O word width, or cache-line size in bits; `512` means a 64-byte cache line. |
| `-Associativity (for cache only)` | Number of cache ways, such as `16`. |
| `-CacheAccessMode` | `Normal`, `Sequential`, or `Fast`; changes how the report combines data and tag paths. |
| `-ProcessNode` | CMOS technology-node label in nm; available nodes are listed under [transistor technology](models/transistor-technology.md). |
| `-DeviceRoadmap` | `HP`, `LSTP`, or `LOP`. The supplied advanced-node SRAM examples use `LOP`; `HP` is unsupported below 22 nm. |
| `-Temperature (K)` | Operating temperature. CMOS helper tables accept 300-400 K; sub-22 nm on-current calibration is limited to 300 K. |
| `-MemoryCellInputFile` | Cell-file path relative to the process working directory. |

The cache access mode determines how tag and data latencies are combined, as
described in the [functional model](functionality.md). With a `RAM` target,
the evaluation includes only the memory array. The storage technology is
selected separately through the cell definition.

## Search objective and organization

`-OptimizationTarget` accepts `ReadLatency`, `WriteLatency`,
`ReadDynamicEnergy`, `WriteDynamicEnergy`, `ReadEDP`, `WriteEDP`,
`ReadBandwidth`, `WriteBandwidth`, `LeakagePower`, and `Area`. `Full` selects
full exploration. The inherited parser also maps unrecognized target strings
to full exploration; the resolved objective is printed at the start of a run.

`-BufferDesignOptimization` selects the `latency`, `balanced`, or `area`
driver-sizing objective. `-EnablePruning: Yes` enables pruning of the
full-exploration results. This pruning path is inactive for single-objective
exploration.

Organization constraints follow the Bank -> SubArray -> Mat -> cell hierarchy.
The correspondence with the inherited DESTINY terminology is given in the
[architecture description](architecture.md).

| Option | Constrained quantity |
| --- | --- |
| `-ForceBankA (Total AxB)` | Total subarray rows and columns per bank; active counts remain search choices. |
| `-ForceBank (Total AxB, Active CxD)` | Total and active subarray rows/columns, for example `1x1, 1x1`. |
| `-ForceSubArrayA (Total AxB)` | Total mat rows and columns within each subarray. |
| `-ForceSubArray (Total AxB, Active CxD)` | Total and active mat rows/columns, for example `1x1, 1x1`. |
| `-ForceMatSize (Rows x Columns)` | Exact logical cell rows/columns required of data-mat candidates. |
| `-ForceMuxSenseAmp`, `-ForceMuxOutputLev1`, `-ForceMuxOutputLev2` | Fix the sense/output multiplexing factors. |

Mat rows and columns are derived from capacity, activation counts, word width,
and multiplexing factors. `-ForceMatSize` retains data-array candidates whose
derived dimensions match the specified values. Capacity and addressing remain
determined by the original configuration. Tag-array dimensions are evaluated
independently of this restriction.

For example, the 8 KiB AOS eDRAM configuration contains one 512 x 128
one-bit-per-cell mat, giving `512 * 128 = 65,536 bits = 8,192 bytes`.
Its organization constraints are:

```text
-ForceBank (Total AxB, Active CxD): 1x1, 1x1
-ForceSubArray (Total AxB, Active CxD): 1x1, 1x1
-ForceMatSize (Rows x Columns): 512 x 128
-ForceMuxSenseAmp: 1
-ForceMuxOutputLev1: 1
-ForceMuxOutputLev2: 1
```

The remaining capacity, word-width, wiring, and cell parameters are specified
in the [complete configuration](https://github.com/neurosim/NS-Cache/blob/main/config/New_Configs/AOS_eDRAM_demo.cfg).

## Geometry, settling, and monolithic tiers

| Option | Default | Definition |
| --- | --- | --- |
| `-RelaxSRAMCell` | `true` | Controls cell-dimension relaxation used to accommodate peripheral geometry; see the [technology page](models/transistor-technology.md). |
| `-BankAspectRatioLimit` | `3` | `0` disables the bank shape check; otherwise a finite value >= 1. Applies to both routing modes. |
| `-ForceMatSize (Rows x Columns)` | Unforced | Two positive 64-bit integers separated by `x` or `X`; infeasible dimensions yield no matching candidate. |
| `-DRAMTargetResidualRatio` | `0.10` | Finite value strictly between 0 and 1 for DRAM/eDRAM restore/write settling and gcDRAM write settling. Smaller values request tighter settling. |
| `-M3DMemory` | `false` | Enables the NS-Cache monolithic mat layout model. |
| `-LimitMonolithicTier (N)` | `4` | Positive integer within the supported `int` range. Sets a maximum tier count; doubling of tiers limits a ceiling of 3 to at most 2 tiers. |

`-RelaxSRAMCell` and `-M3DMemory` accept case-insensitive `yes/no`, `true/false`,
and `1/0`. Older flags retain their individual parsing rules. The `(N)` suffix
is part of the required tier-limit key.

The [monolithic 3D model](models/monolithic-3d.md) operates at mat level.
`-StackedDieCount` and `-PartitionGranularity` describe the inherited die-level
organization, while `-LocalTSVProjection` and `-GlobalTSVProjection` select TSV
parameters. `-TSVRedundancy` also scales the MIV count in the monolithic model.

## Wiring, sensing, and report controls

| Option | Definition |
| --- | --- |
| `-Routing` | `H-tree` or `Non-H-tree`; selects the bank routing class. |
| `-InternalSensing` | The case-sensitive value `true` enables internal sensing. External sensing support depends on the cell and routing model. |
| `-LocalWireType`, `-GlobalWireType` | Examples use `LocalAggressive` and `GlobalAggressive`; conservative alternatives are also parsed. |
| `-LocalWireRepeaterType`, `-GlobalWireRepeaterType` | `RepeatedNone` selects passive wires; `RepeatedOpt` requests the optimal-repeater model. |
| `-LocalWireUseLowSwing`, `-GlobalWireUseLowSwing` | Case-sensitive `Yes` enables low swing; `No` disables it. |
| `-ViewMatStatistics` | Presence of the key enables detailed mat output, including when its value is `false`. Omission disables it. |
| `-ViewQuantization` | Presence of the key enables cache-level cycle and architectural reporting. Omission disables it. |
| `-ClockFrequency` | Frequency in Hz for cycle quantization; default `3e9`. It does not change the underlying continuous-time circuit delays. |
| `-PrintAllOptimals` | In a single-objective run, exact `true` prints the best result for every optimization target. |

`-NSWiring: true` remains in some example files but is unrecognized by the
current parser and has no effect on wiring selection. Unrecognized legacy
keys may be ignored without an error. Resolved settings and the selected
organization are included in the console report.

In `Full` exploration, the current executable derives the CSV filename from
the input configuration path by replacing its extension with `.csv`. It can
overwrite an earlier CSV at that path. Although `-OutputFilePrefix` is parsed,
the filename is assigned in `main.cpp` independently of that value.
Single-objective reports are written to standard output, as shown in
[Running NS-Cache](getting-started.md).

## Cell files

Common `.cell` entries include `-MemCellType`, `-CellArea (F^2)`,
`-CellAspectRatio`, sensing settings, access-device sizing, and voltages.
DRAM-family definitions add capacitance and retention; SRAM definitions add
the cell's transistor widths. Each memory technology therefore requires its
corresponding set of electrical and geometric parameters; changing
`-MemCellType` alone does not define the new cell.

For oxide devices, `-OxideTransistor: true` enables a stricter schema with
separate access or read/write prefixes. The [AOS overview](models/aos.md)
describes those paths, explicit device dimensions, mobility choices, and the
limits of the supplied examples.

Parameter parsing and defaults are implemented in
[InputParameter.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/InputParameter.cpp),
[MemCell.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/MemCell.cpp), and
[main.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/main.cpp).
