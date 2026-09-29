# Peripheral circuits

These blocks inherit `FunctionUnit`. Calculations write object fields; they
return no metrics directly. `Mat`, `SubArray`, and the non-H-tree bank select
and scale the blocks.

## Shared calculation interface

All eight classes provide:

```cpp
void CalculateArea();
void CalculateRC();
void CalculateLatency(double _rampInput);
void CalculatePower();
void PrintProperty();
```

| Call | Stored output |
| --- | --- |
| `Initialize(...)` | Circuit sizes, loads, initialization state. |
| `CalculateArea()` | Width/height in m; area in m². |
| `CalculateRC()` | Capacitances in F; resistances in ohms. |
| `CalculateLatency(ramp)` | Delays in s; output slope where supported. See [timing units](index.md#functions-and-units). |
| `CalculatePower()` | Dynamic energy in J; leakage in W. |
| `PrintProperty()` | Prints current fields; performs no calculation or validation. |

!!! note "Call order"
    Initialize first, then prepare area/RC before latency/power. Follow the
    owner's sequence: some owners prepare RC during initialization.

!!! warning "Caller checks"
    Constructors leave circuits uninitialized. Calculation calls usually print
    an error and return if initialization is missing. Reinitialization commonly
    warns and continues; use fresh candidates. Supply valid global technology,
    temperature, counts, and loads: most methods do not fully check these inputs.

Listed assignment operators return `*this` without recalculation. Each class
below lists what it copies.

## `BasicDecoder`

Decodes 1–3 address bits. **Owns:** `OutputDriver` by value.
**Caller:** `PredecodeBlock`. **Globals:** `tech`, `inputParameter`.
[Header](https://github.com/neurosim/NS-Cache/blob/main/src/BasicDecoder.h) ·
[Source](https://github.com/neurosim/NS-Cache/blob/main/src/BasicDecoder.cpp)

```cpp
void Initialize(int _numAddressBit, double _capLoad, double _resLoad,
                double _wireLength);
```

Inputs: 1–3 bits, load in F/ohms, length in m. Stores loads, sizes NANDs,
and initializes a `latency_first` driver. One bit uses the inverter path.
The bit range is **not checked**; this class has no `invalid` flag.

| Call | Behavior |
| --- | --- |
| Area | Decoder/driver layout for all output gates. |
| RC | Driver RC and NAND input/output capacitance. |
| Latency | Decode plus driver delay; equal read/write latency and output slope. |
| Power | One active path's energy; all output gates/drivers' leakage. |

## `PredecodeBlock`

Builds a multistage decoder. **Owns:** allocated `BasicDecoder`/`RowDecoder`
stages; constructor nulls pointers, destructor deletes them.
**Caller:** `SubArray` (eight blocks). **Globals:** child circuits' technology,
configuration, and cell dependencies.
[Header](https://github.com/neurosim/NS-Cache/blob/main/src/PredecodeBlock.h) ·
[Source](https://github.com/neurosim/NS-Cache/blob/main/src/PredecodeBlock.cpp)

```cpp
void Initialize(int _numAddressBit, double _capLoad, double _resLoad);
PredecodeBlock &operator=(const PredecodeBlock &rhs);
```

Inputs: address bits, load in F/ohms. Allocates stages, stores counts/loads,
and prepares row-stage RC for earlier-stage sizing.

| Call | Behavior |
| --- | --- |
| Area | Combined stage footprint. |
| RC | Updates basic-decoder RC; row-stage RC is ready from initialization. |
| Latency | Combines parallel paths and serial stages; read/write delay and output slope. |
| Power | Active-path energy and all-stage leakage. |

Zero bits bypasses decoding: zero metrics and unchanged input slope.
More than 27 bits prints an error and exits. Negative counts are not checked.

!!! warning "Copying and reuse"
    Assignment copies selected metrics/scalars, **not stage
    pointers**. Use the copy for reporting, not recalculation. Reinitialization
    does not fully free earlier allocations.

## `RowDecoder`

Drives row or mux selection. **Owns:** `OutputDriver` by value.
**Callers:** `Mat`, `PredecodeBlock`.
**Globals:** `tech`, `inputParameter`, `cell` for DRAM-family power.
[Header](https://github.com/neurosim/NS-Cache/blob/main/src/RowDecoder.h) ·
[Source](https://github.com/neurosim/NS-Cache/blob/main/src/RowDecoder.cpp)

```cpp
void Initialize(int _numRow, double _capLoad, double _resLoad,
                bool _multipleRowPerSet, BufferDesignTarget _areaOptimizationLevel,
                double _minDriverCurrent, bool _MUX, double _wireLength);
RowDecoder &operator=(const RowDecoder &rhs);
```

Inputs: positive row count, loads in F/ohms, current in A, length in m.
`_multipleRowPerSet` enables extra way selection. Stores the requested target
but sizes the driver with `latency_first`; forwards `_MUX` as `_addRepeaters`.
Sizes NANDs/driver. Driver failure sets `invalid` and returns before initialization.

| Call | Behavior |
| --- | --- |
| Area | Decoder/driver footprint × rows. |
| RC | Driver RC; NAND capacitances, or zero for the direct-driver path. |
| Latency | Equal read/write delay and output slope. |
| Power | One row's energy; all rows' leakage. DRAM-family NAND power uses `vpp`. |

Calculations check initialization, not driver validity again. Assignment copies
selected state and the driver.

## `OutputDriver`

Sizes an inverter chain. **Owns:** fixed width/capacitance arrays.
**Callers:** decoders, prechargers. **Globals:** `tech`, `inputParameter`.
[Header](https://github.com/neurosim/NS-Cache/blob/main/src/OutputDriver.h) ·
[Source](https://github.com/neurosim/NS-Cache/blob/main/src/OutputDriver.cpp)

```cpp
void Initialize(double _logicEffort, double _inputCap, double _outputCap,
                double _outputRes, bool _inv,
                BufferDesignTarget _areaOptimizationLevel,
                double _minDriverCurrent, bool _addRepeaters, double _wireLength);
OutputDriver &operator=(const OutputDriver &rhs);
```

Requires positive effort/input capacitance and valid technology/current tables.
Capacitances: F; resistance: ohms; current: A; length: m. `_inv` selects inversion
parity; the target controls sizing. Writes `numStage`, widths, loads, and flags.
Infeasible drive sets `invalid` and returns. Oversized latency-first chains may
be limited or use drive-constrained sizing.

| Call | Behavior |
| --- | --- |
| Area | Sum of stage layouts; invalid chains get markers for dimensions/area. |
| RC | Stage capacitances; zero stages sets `capInput[0] = 0`; invalid chains skip. |
| Latency | Chain/load delay and output slope; invalid chains get markers. |
| Power | Chain energy/leakage; invalid chains get markers. |

Zero-stage non-inverting chains have zero area/energy/delay and pass the slope
through. Assignment copies selected fields/arrays.

!!! note "Repeaters"
    `_addRepeaters` and length are stored, but local repeater code is commented
    out. Active repeaters are handled by `Mat::CalculateRepeater` and `Wire`.

## `Precharger`

Precharges/equalizes bitlines. **Owns:** enable `OutputDriver` by value.
**Caller:** `Mat` (`precharger` and gcDRAM `writeDriver`).
**Globals:** `tech`, `cell`, `localWire`, `inputParameter`.
[Header](https://github.com/neurosim/NS-Cache/blob/main/src/Precharger.h) ·
[Source](https://github.com/neurosim/NS-Cache/blob/main/src/Precharger.cpp)

```cpp
void Initialize(double _voltagePrecharge, int _numColumn,
                double _capBitline, double _resBitline, double _wireLength);
Precharger &operator=(const Precharger &rhs);
```

Inputs: voltage in V, positive column count, load in F/ohms, length in m.
Requires cell width and initialized local-wire parasitics. Sizes devices,
column/control loads, and driver. No `invalid` flag.

| Call | Behavior |
| --- | --- |
| Area | Column circuits plus driver footprint. |
| RC | Driver RC; column capacitances are ready from initialization. |
| Latency | Separate `enableLatency`; bitline charging in read/write/refresh latency; output slope. |
| Power | Control/driver read energy and leakage; write energy zero, refresh energy equals read energy. Mat adds bitline/cell energy. |

The gcDRAM mat uses `writeDriver.readDynamicEnergy` for writes.
Assignment copies selected metrics, loads, sizes, and driver.

## `SenseAmp`

Models parallel sense amplifiers and optional current-to-voltage conversion.
**Owns:** scalar state and binary records by value.
**Callers:** `Mat`, non-H-tree external sensing, low-swing `Wire`.
**Globals:** `tech`, `inputParameter`.
[Header](https://github.com/neurosim/NS-Cache/blob/main/src/SenseAmp.h) ·
[Source](https://github.com/neurosim/NS-Cache/blob/main/src/SenseAmp.cpp)

```cpp
void Initialize(long long _numColumn, bool _currentSense,
                double _senseVoltage, double _pitchSenseAmp,
                const BinarySenseAmpOperatingPoint *operatingPoint = nullptr);
bool UsesBinaryModel() const;
SenseAmp &operator=(const SenseAmp &rhs);
```

Inputs: amplifier count, current-sensing flag, swing in V, pitch in m.
Copies the optional point (null gives defaults); caller retains ownership.
Resets `invalid`/`binaryResult`, stores inputs, and evaluates current-sensing
points. `Fallback` uses legacy sensing; `Invalid` prints a diagnostic and sets
`invalid`. Pitch ≤ three feature sizes also sets `invalid`. Both failures
still set `initialized`.

`UsesBinaryModel()` returns current sensing with status `Applied`; no state
change or initialization/pitch check. `Mat` supplies a point; external sensing
omits it. Low-swing wire receivers use voltage sensing.

| Call | Behavior |
| --- | --- |
| Area | Binary total area, width = pitch × count, derived height; otherwise legacy layout/converter. Invalid state gets markers. |
| RC | Binary per-port capacitance or legacy `capLoad`. Invalid state sets read/write latency markers. |
| Latency | Binary saved delay or legacy delay; read = refresh, write = zero; ramp unused. Binary `Invalid` gets read/write/refresh markers. |
| Power | Total read/refresh energy and leakage; write energy zero. Invalid state sets read/write energy and leakage markers. |

!!! warning "Before calculation"
    Reject `invalid` amplifiers: latency does not separately guard pitch failure.
    Legacy paths require positive counts and meaningful swing below supply;
    these are caller checks. Binary totals already include amplifier count.

Assignment copies metrics and both records. Reinitialize
after changing technology or the operating point; saved results are not recomputed.

### Binary sense-amplifier interfaces

Value records and pure free functions. `SenseAmp::Initialize` stores the evaluator
result. No owned resources, global reads, or argument changes.
[Header](https://github.com/neurosim/NS-Cache/blob/main/src/BinarySenseAmp.h) ·
[Source](https://github.com/neurosim/NS-Cache/blob/main/src/BinarySenseAmp.cpp)

```cpp
BinarySenseAmpResult EvaluateBinarySenseAmp(
    const BinarySenseAmpOperatingPoint& point,
    const Technology& technology, long long numAmplifiers);
```

Requires initialized technology and positive count.

| Operating-point fields | Meaning; default |
| --- | --- |
| `contextAvailable`, `internalSenseAmp` | Concrete context and local sensing; false. |
| `tierCount`, `cellType`, `accessType`, `readPowerOverride` | Eligibility; 1, MRAM, CMOS access, false. |
| `temperatureK`, `readVoltage` | K, V; 300, 0. |
| `resistanceOn`, `resistanceOff` | Ohms, including access resistance; 0. |
| `columnCapacitance` | F, external column/access load; excludes amplifier inputs and mux; 0. |
| `referenceColumnArea` | m², one full-height dummy column; 0. |

| Returned `status` | Caller action |
| --- | --- |
| `Applied` | Use metrics. `reason` is `neurosim-binary-adaptation`. |
| `Fallback` | Run legacy sensing; `reason` identifies unsupported/missing context. |
| `Invalid` | Reject candidate; `reason` identifies bad inputs or nonphysical results. |

`Applied` requires internal sensing, one tier, MRAM/PCRAM/memristor, CMOS access,
no read-power override, LSTP, 300 K, and an exact node: 90, 65, 45, 32, 22, 14,
10, 7, 5, or 3 nm. Bias must match nominal within `1e-12` V; normalized resistance
margin must be ≥ 0.10. Unsupported settings return `Fallback`.

!!! warning "Validation order"
    Missing/external context returns `Fallback` first. With internal context,
    invalid count, resistance, column capacitance, reference area, temperature,
    or bias returns `Invalid` before eligibility checks. Resistances must be
    finite/positive with off ≥ on; capacitance/area/temperature finite/positive;
    bias finite/nonnegative. Technology initialization is checked after
    tier/cell/access/override checks. Latency, input capacitance, and core area
    must be finite/positive; energy and power finite/nonnegative. Failed checks
    return `Invalid` with zero numeric fields.

| Result fields | Units and scope |
| --- | --- |
| `referenceResistance`, `resistanceMargin`, `nominalReadVoltage` | Ohms, dimensionless, V. |
| `inputCapacitance` | F per data port; reference port separate. |
| `gateCapacitanceN/P`, `junctionCapacitanceN/P` | F per device; not scaled by count. |
| `readLatency` | s per parallel event; excludes wire/mux delay. |
| `coreArea`, `referenceArea`, `area` | m², totals including amplifier count. |
| `columnSwitchingEnergy`, `internalSwitchingEnergy`, `operatingEnergy`, `readDynamicEnergy` | J, component/total energies including count. Column term includes data/reference columns. |
| `leakage` | W, total analytical standby estimate. |
| `perAmplifierArea`, `perAmplifierReadDynamicEnergy`, `perAmplifierLeakage` | m², J, W per amplifier. |

Defaults: `Fallback`, empty reason, zero metrics.

!!! note "Model limits and accounting"
    Experimental transfer: calibration envelope unpublished; standby leakage
    uncalibrated. Do not count included cell/column and amplifier-input energy
    twice. `Mat` handles this when the binary model applies.

Use the fit functions to check numerical values without evaluating a full circuit:

```cpp
double BinarySenseAmpNominalReadVoltage(int nodeNm);
double BinarySenseAmpLatencyFit(int nodeNm, double resistance,
                               double columnCapacitance);
double BinarySenseAmpOperatingPowerFit(int nodeNm, double resistance,
                                      double readVoltage,
                                      double temperatureK = 300);
```

Returns: V, s, W respectively. Inputs: nm, ohms, F, V, K.
Unsupported nodes or malformed inputs return quiet NaN. Latency requires finite
positive resistance and finite nonnegative capacitance. Power requires finite
positive resistance, bias, temperature, and bias-normalized resistance.
No count scaling, layout, reference selection, or evaluator eligibility checks.
Negative extrapolated results can be returned; callers must check them.

## `Mux`

Models parallel pass-transistor muxes. **Owns:** scalar device/load state.
**Callers:** `Mat`, non-H-tree global bitlines.
**Globals:** `tech`, `inputParameter`, `cell`.
[Header](https://github.com/neurosim/NS-Cache/blob/main/src/Mux.h) ·
[Source](https://github.com/neurosim/NS-Cache/blob/main/src/Mux.cpp)

```cpp
void Initialize(int _numInput, long long _numMux, double _capLoad,
                double _capInputNextStage, double _minDriverCurrent);
Mux &operator=(const Mux &rhs);
```

Inputs: inputs per mux, parallel count, two loads in F, current in A.
Stores inputs; sizes devices when `numInput > 1 && numMux > 0`.
Other counts bypass; no general count check or `invalid` flag.

| Call | Behavior |
| --- | --- |
| Area | Active footprint; bypass gives zero dimensions/area. |
| RC | Device RC, `capOutput`, `capForPreviousDelayCalculation`, `capForPreviousPowerCalculation`; bypass skips. |
| Latency | Equal read/write delay; ramp stored but unused; no useful output slope. Bypass gives zero. |
| Power | Read/write energy × mux count; leakage zero; bypass energy zero. |

Assignment copies selected fields. Initialize and prepare
RC again for a new path.

## `Comparator`

Compares tags in four quarters. **Owns:** fixed device arrays and scalar loads.
**Callers:** internal tag subarrays, non-H-tree external tags.
**Globals:** `tech`, `inputParameter`.
[Header](https://github.com/neurosim/NS-Cache/blob/main/src/Comparator.h) ·
[Source](https://github.com/neurosim/NS-Cache/blob/main/src/Comparator.cpp)

```cpp
void Initialize(int _numTagBits, double _capLoad);
Comparator &operator=(const Comparator &rhs);
```

Inputs: tag bits, load in F. Stores integer `_numTagBits / 4` and sizes devices.
Tag width must be divisible by four: **not checked or padded**. No `invalid` flag.

| Call | Behavior |
| --- | --- |
| Area | Adjusts sizes; lays out four quarters. |
| RC | Inverter capacitances; `capBottom`, `capTop`, `resBottom`, `resTop`. Run after area. |
| Latency | Equal read/write delay and output slope; parent chooses applicable operations. |
| Power | Equal read/write energy; total leakage of four quarters. |

Assignment copies selected scalars/arrays.

Next: [Interconnects](interconnects.md).
