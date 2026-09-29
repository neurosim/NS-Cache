# Array hierarchy

```text
Bank → SubArray → Mat → local peripherals
```

See the [call order](index.md#example-trace-a-supplied-sram-configuration) and
[model organization](../architecture.md#physical-hierarchy-and-code-ownership).

## `FunctionUnit`

Base metrics for arrays, peripherals, and TSVs. No ownership or globals.
Constructor zeros fields; destructor is virtual.

Sources: [FunctionUnit.h](https://github.com/neurosim/NS-Cache/blob/main/src/FunctionUnit.h),
[FunctionUnit.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/FunctionUnit.cpp).

| Fields | Units / meaning |
| --- | --- |
| `height`, `width`, `area` | m, m, m² |
| `readLatency`, `writeLatency`, `setLatency`, `resetLatency`, `refreshLatency` | s; optional operations depend on the cell |
| `readDynamicEnergy`, `writeDynamicEnergy`, `setDynamicEnergy`, `resetDynamicEnergy`, `refreshDynamicEnergy` | J per operation at this level |
| `cellReadEnergy`, `cellSetEnergy`, `cellResetEnergy` | J; cell contributions aggregated by the owner |
| `leakage` | W; DRAM-family totals include a refresh proxy |

```cpp
virtual void PrintProperty();
virtual void MagicLayout();
virtual void OverrideLayout();
int logical_effort(int num_gates_min, double g, double F,
                   double *w_n, double *w_p, double C_load,
                   double p_to_n_sz_ratio, double max_w_nmos,
                   Technology tech);
```

| Method | Behavior |
| --- | --- |
| `PrintProperty` | Prints current metrics to `cout`; no calculation or validation. |
| `MagicLayout` | Requires calculated `area` and positive `newHeight` or `newWidth` (m). Reshapes at constant area. Height takes priority; both zero means no change. Positivity is unchecked. |
| `OverrideLayout` | Sets both requested dimensions and replaces `area`. Exits if either dimension is zero; positivity is unchecked. |
| `logical_effort` | TSV buffer sizing: dimensionless efforts/ratios, `C_load` in F, widths in m, initialized `tech`. Caller provides writable arrays with stage zero set and room for `MAX_NUMBER_GATES_STAGE`. Writes later widths; returns stage count. Positive inputs are unchecked; stage limit is asserted after sizing. |

Layout helpers are used by the inactive `LevelShifter`.

## `Bank` and routing implementations

Abstract bank model; owns `subarray` and `tsvArray` by value. Search allocates
the class selected by `inputParameter->routingMode`.
Globals: `inputParameter`, `tech`, `cell`, `localWire`, `globalWire`; children
also use `devtech`.

Sources: [Bank.h](https://github.com/neurosim/NS-Cache/blob/main/src/Bank.h),
[Bank.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Bank.cpp),
[BankWithHtree.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/BankWithHtree.cpp),
[BankWithoutHtree.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/BankWithoutHtree.cpp).

### Initialization

Both routing classes implement:

```cpp
void Initialize(int _numRowSubArray, int _numColumnSubArray, long long _capacity,
                long _blockSize, int _associativity, int _numRowPerSet,
                int _numActiveSubArrayPerRow, int _numActiveSubArrayPerColumn,
                int _muxSenseAmp, bool _internalSenseAmp,
                int _muxOutputLev1, int _muxOutputLev2,
                int _numRowMat, int _numColumnMat,
                int _numActiveMatPerRow, int _numActiveMatPerColumn,
                BufferDesignTarget _areaOptimizationLevel, MemoryType _memoryType,
                int _stackedDieCount, int _partitionGranularity,
                int monolithicStackCount);
```

| Inputs | Meaning |
| --- | --- |
| Row/column counts | Physical grids of subarrays and mats |
| `_capacity`, `_blockSize` | Storage and access width, **bits** |
| `_associativity`, `_numRowPerSet` | Effective ways; rows per set |
| Active counts | Active blocks; “per row” is bounded by columns, and vice versa |
| Mux factors | Bitlines per amplifier; two output-selection ratios |
| `_internalSenseAmp` | Local sensing; required for H-tree |
| `_areaOptimizationLevel` | Buffer target; some peripherals force latency-first sizing |
| `_memoryType` | `MemoryType::data` or `MemoryType::tag` |
| `_stackedDieCount`, `_partitionGranularity` | Stacked dies; coarse/fine partitioning |
| `monolithicStackCount` | Mat `_num3DLevels`; separate from `monolithic3DMat` folding |

Requires initialized globals and compatible positive counts/widths/mux factors.
Stores organization, derives address/routing widths, initializes children.
Base declaration spelling: `_paritionGranularity`.

| Implementation | Owned state / enforced limits |
| --- | --- |
| `BankWithHtree` | Allocated route-count/length arrays; freed on destruction and reinitialization. Rejects external sensing. |
| `BankWithoutHtree` | Value-owned `globalSenseAmp`, `globalBitlineMux`, `globalComparator`. Calculates subarray area during initialization to size external sensing. Rejects external sensing for DRAM/eDRAM/gcDRAM or repeated global wires. |

!!! warning "Invalid input"
    Active counts above physical counts are clamped with warnings. Bad partitioning,
    insufficient ways, or invalid children set `invalid`; some also set `initialized`.
    Integer inputs are not fully checked. Search mostly uses powers of two.

### Evaluation and copying

```cpp
void CalculateArea();
void CalculateRC();
void CalculateLatencyAndPower();
void PrintProperty();
virtual Bank &operator=(const Bank &rhs);
BankWithHtree &operator=(const BankWithHtree &rhs);
BankWithoutHtree &operator=(const BankWithoutHtree &rhs);
```

| Method | Requires → writes / checks |
| --- | --- |
| `CalculateArea` | Initialization → geometry, route/child/TSV area. Rejects invalid children and enabled data-bank aspect-ratio violations; uses area markers. |
| `CalculateRC` | Area → subarray RC and external non-H-tree peripheral RC. No separate H-tree route-capacitance pass. |
| `CalculateLatencyAndPower` | Area + RC → child timing, then power, bank totals, routing, TSV traffic. Rejects invalid children or refresh time beyond retention. |
| `PrintProperty` | Prints saved common metrics. |
| Assignment | Returns `*this`; copies selected metrics, organization, routing, subarray, and TSV state. Concrete overloads add selected derived fields. |

!!! warning "Saved banks are reporting state"
    Assignment through `Bank*` uses `operator=(const Bank&)`. Concrete overloads
    do not override it. Neither path rebuilds all routing internals; H-tree arrays
    are not cloned. Start a fresh initialization before reevaluation.

## `SubArray`

Called by both banks. Owns one `Mat`, eight predecoders, a tag comparator, and
a TSV. Uses global configuration, cell, technology, and local-wire state.

Sources: [SubArray.h](https://github.com/neurosim/NS-Cache/blob/main/src/SubArray.h),
[SubArray.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/SubArray.cpp).

```cpp
void Initialize(int _numRowMat, int _numColumnMat, int _numAddressBit,
                long _numDataBit, int _numWay, int _numRowPerSet, bool _split,
                int _numActiveMatPerRow, int _numActiveMatPerColumn,
                int _muxSenseAmp, bool _internalSenseAmp,
                int _muxOutputLev1, int _muxOutputLev2,
                BufferDesignTarget _areaOptimizationLevel, MemoryType _memoryType,
                int _stackedDieCount, int _partitionGranularity,
                int monolithicStackCount);
void CalculateArea();
void CalculateRC();
void CalculateLatency(double _rampInput);
void CalculatePower();
void PrintProperty();
SubArray &operator=(const SubArray &rhs);
```

Inputs: mat counts, address/data widths (bits), distributed ways, bank
activation/mux/stack settings. Requires initialized globals and compatible positive
organization. `_split` is stored; the mat receives `true`.

| Method | Writes / checks |
| --- | --- |
| `Initialize` | Derives mat dimensions, initializes mat and its area, then predecoders. Initializes comparator for tag/internal sensing and TSVs for fine stacked partitioning. Rejects bad dimensions, address shortage, invalid mat, or forced data-mat size mismatch. Forced dimensions filter data mats; they do not resize them or constrain tag mats. |
| `CalculateArea` | Uses mat footprint; calculates predecoder/comparator/TSV area. Writes dimensions, `area`, `areaAllLogicBlocks`. |
| `CalculateRC` | Predecoder/comparator RC; mat RC already exists. |
| `CalculateLatency` | Input slope + RC → predecoder/mat/TSV/comparator timing, `predecoderLatency`, operation latencies. Propagates mat invalidity. |
| `CalculatePower` | Timing → energies/leakage; active mats scale access energy, physical mats scale leakage. Adds refresh/tag terms; propagates mat invalidity. |
| `PrintProperty` | Prints saved metrics to `cout`. |
| Assignment | Returns `*this`; copies selected state/children. Predecoder stages are not cloned. |

!!! note "Reinitialization"
    Reinitialization warns and does not fully reset allocated predecoder stages.

## `Mat`

Called by `SubArray`. Owns row/mux decoders, precharger, sensing/mux stages,
gcDRAM read decoder/write driver, and monolithic-via model. Cells use shared
`MemCell` parameters. Globals: `tech`, `devtech`, `cell`, `localWire`, `inputParameter`.

Sources: [Mat.h](https://github.com/neurosim/NS-Cache/blob/main/src/Mat.h),
[Mat.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Mat.cpp).

```cpp
void Initialize(long long _numRow, long long _numColumn,
                bool _multipleRowPerSet, bool _split,
                int _muxSenseAmp, bool _internalSenseAmp,
                int _muxOutputLev1, int _muxOutputLev2,
                BufferDesignTarget _areaOptimizationLevel, int _num3DLevels,
                int _stackedDieCount = 1);
void CalculateArea();
void CalculateLatency(double _rampInput);
void CalculatePower();
void CalculateRepeater(int numCol);
void PrintProperty();
Mat &operator=(const Mat &) = default;
```

Inputs: row/column and mux counts, set/decoder-split flags, sensing mode,
buffer target, monolithic levels, stacked dies. `SubArray` forwards both stack counts.

| Method | Requires → writes / checks |
| --- | --- |
| `Initialize` | Global context → resets result structures, selects cell/sensing path, computes wire/load/repeater state, initializes peripherals **and RC**, caches AOS operating points. Checks positive rows/columns/mux/levels, cell sensing/row limits, driver feasibility, and child validity. DRAM/eDRAM/gcDRAM reject `muxSenseAmp > 1`; MLC NAND is unfinished and rejected. |
| `CalculateArea` | Initialization → geometry, folding/MIV counts, `stackedMemTiers`, `m3d`. Bad geometry/tiers or count overflow set `invalid` and the area marker. Run before timing. |
| `CalculateLatency` | Layout + RC + input slope → operation/column/bitline delays, `dramTiming`. DRAM-family checks may invalidate. DRAM/eDRAM read latency excludes restoration. |
| `CalculatePower` | Layout + timing → energy/leakage/refresh totals, `gcDramPower`, `aosLeakageUpperBound`; some paths set global cell write energy. Rejects invalid DRAM-family energies. gcDRAM write-driver energy uses its `Precharger::readDynamicEnergy`. |
| `CalculateRepeater` | Known wordline RC + positive `numCol` → `numRepeaters`, `bufferSizeRatio`; may cache global `optNumRepeaters`/`optSizeRepeaters` at `int(log2(numCol))`. Index bounds are unchecked. Disabled: count 0, ratio 1. Does not rebuild peripherals. |
| `PrintProperty` | Prints evaluated dimensions, areas, efficiency, delays. |
| Default assignment | Copies values/peripherals/results; global cell/technology stay unchanged. |

Failures set `invalid` and return, sometimes with a diagnostic. Child requirements
still apply.

### Structured result fields

| Field / type | Values / when set |
| --- | --- |
| `dramTiming` / `DRAMTimingResult` | DRAM/eDRAM timing: `accessLatency`, `restoreDelay`, `readCycleLatency`, `writeBitlineDelay` in s. Read cycle includes restoration and drives bandwidth. |
| `gcDramPower` / `GcDRAMPowerResult` | gcDRAM power: `readBitlineAccessEnergy`, `writeBitlineAccessEnergy`, `writeDriverEnergy` in J per mat; `aosLeakageUpperBound` in W. |
| `m3d` / `M3DLayoutResult` | Layout: 64-bit `mivsPerTier`, `totalMivCount`; `totalMivArea`, `peripheralLogicArea`, `finalLogicLayerArea`, `perTierMemoryArea`, `projectedArea` in m²; `dominantTier` = `none`, `logic`, or `memory`. |
| `aosAccessOperatingPoint`, `aosReadOperatingPoint`, `aosWriteOperatingPoint` | Initialization: cached AOS current/resistance/capacitance; [device fields](devices.md#aosfetcompactmodel). |

!!! note "Read results after evaluation"
    Timing and power calculations reset their own result structures.
    Read fields only for the matching cell model.
    See [refresh/leakage interpretation](../functionality.md).

Next: [Peripheral circuits](peripherals.md).
