# Results and reporting

`Result` selects candidates and reports saved metrics. Callers: `main`, `nvsim`,
and search macros. It does not inherit `FunctionUnit` or reevaluate circuits.

Sources: [Result.h](https://github.com/neurosim/NS-Cache/blob/main/src/Result.h),
[Result.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Result.cpp).

## Ownership

| State | Ownership and use |
| --- | --- |
| `bank`, `localWire`, `globalWire` | Constructor allocates; destructor deletes. Global `inputParameter->routingMode` selects the bank class. Wires are separate from global wire objects. |
| Defaults | Main metrics = `invalid_value`; limits unconstrained; objective = read latency. |
| `cellTech` | Borrows global `cell`. Keep it alive for queries/reports, including after a cell sweep. |
| Global dependencies | Reports still use `inputParameter` and some `tech` geometry ratios. Keep them consistent with the saved result. |

!!! warning "Do not copy Result by value"
    Implicit copying shares owned pointers. Use `compareAndUpdate` to save bank/wire
    state; [bank copies](hierarchy.md#evaluation-and-copying) are for reporting.

## Reset and comparison

```cpp
void reset();
bool compareAndUpdate(Result &newResult);
string printOptimizationTarget();
```

| Method | Input → output / state changes |
| --- | --- |
| `reset` | Sets read/write latency/energy, leakage, dimensions, and area to invalid markers. Keeps objective, limits, cell, wires, and other child state. Returns `void`. |
| `compareAndUpdate` | Evaluated candidate → `true` if it meets the current result's limits and improves its objective. Copies bank/wires and borrowed `cellTech`; keeps limits/objective. Otherwise returns `false` unchanged. Ties keep the old result; candidate is unchanged. |
| `printOptimizationTarget` | Returns objective label; prints nothing and changes no state. |

Limits cover latency (s), energy (J), EDP (J·s), area (m²), leakage (W), and
bandwidth (bytes/s). Bandwidth limits are minimums; others are maximums.

!!! warning "Check candidates first"
    `compareAndUpdate` does not check `bank->invalid`.
    Use an ordinary optimization target: `full_exploration` selects no winner
    and also serves as the result-array size bound.

## Bandwidth queries

```cpp
double getReadBandwidth() const;
double getWriteBandwidth() const;
```

Return `bank->blockSize / cycle / 8` in **bytes/s**. Require a live saved cell
and evaluated timing; leave state unchanged.

| Query | Cycle |
| --- | --- |
| Read, DRAM/eDRAM | `mat.dramTiming.readCycleLatency`, including restoration |
| Read, gcDRAM | `mat.readLatency - mat.gcRowDecoder.readLatency + mat.precharger.readLatency` |
| Read, other cells | `mat.readLatency - mat.rowDecoder.readLatency + mat.precharger.readLatency` |
| Write | `mat.writeLatency` |

Return zero for a missing cell, nonpositive block size, or a nonpositive cycle.
Also return zero if bank latency or cycle is nonfinite or at or above `invalid_value / 10`.

## Array reporting

```cpp
void print(int indent = 0);
void printToFile(int indent = 0, const string &FileName = "out.txt");
void printToStream(int indent, ostream &outFile);
void printToCsvFile(ofstream &outputFile);
```

!!! note "Before calling"
    Evaluate the bank and save its wire choices. These methods do not check invalid
    markers. `indent` is the number of indentation spaces.

| Method | Output / failures |
| --- | --- |
| `print` | `cout`: cell preamble and array report. |
| `printToFile` | Opens/truncates `FileName`; calls `printToStream`. Open failure: diagnostic to `cerr`, then return. |
| `printToStream` | Caller-owned stream; array report without cell preamble. Does not open/close the stream. |
| `printToCsvFile` | Open `ofstream`: positional fields, no header, trailing comma/newline; also a debug line to `cout`. Values use writer-specific scaling, not uniform SI. |

Missing cell prints a diagnostic and returns. Caller checks stream write errors.

### Sensing metadata

Reports select the mat amplifier for internal sensing or non-H-tree
`globalSenseAmp` for external sensing.

| Output | Contents |
| --- | --- |
| Model | `neurosim-binary-v1`, `legacy-current`, or `voltage`; current-mode fallback includes a reason (`external-sensing` for external sensing). |
| Binary status | Calibration/standby status from `PrintSensingDetails`. |
| `viewMatStats` | `CSA SI` metrics, operating point, reference/margin, energy parts, bank/mat totals, mux loads. Labels give m²/F/s/J/W/ohms/V/K; counts/margin are dimensionless. |
| `sensing_path_energy_J` | Internal current sensing: amplifier + cell + column + data-mux energy per mat. |
| CSV suffix | Model, fallback reason (possibly empty), `experimental-uncalibrated-standby` for binary or `legacy`. Text fields follow numeric fields. |

Default binary fields on fallback/voltage paths are not evaluated binary results.

## Cache reporting

Call on the **data** result with the matching **tag** result:

```cpp
void printAsCache(Result &tagResult, CacheAccessMode cacheAccessMode);
void printAsCacheToFile(Result &tagResult, CacheAccessMode cacheAccessMode,
                       const string &FileName);
void printAsCacheToCsvFile(Result &tagResult, CacheAccessMode cacheAccessMode,
                          ofstream &outputFile);
```

Requires evaluated results, live saved cells, and the access mode used in
exploration. Returns `void`; computes output summaries without changing bank
metrics. Wrong data/tag `memoryType` pairing or missing saved data cell produces
a diagnostic and return; callers must validate both results.

| Method | Destination |
| --- | --- |
| `printAsCache` | `cout`; summary, array details, applicable quantized output |
| `printAsCacheToFile` | Opens/truncates file; summary and details. Open failure goes to `cerr`. |
| `printAsCacheToCsvFile` | Caller-owned open stream; summary, both array records, combined mat fields. Caller checks errors. |

!!! warning "Cache CSV spans multiple lines"
    Each array writer adds a newline. The cache writer adds more fields and another
    newline. It is not one CSV row per cache result.

See [cache access modes](../functionality.md#cache-access-modes-and-external-simulation)
for how data and tag metrics are combined. Quantization uses `clockFreq` (Hz) and
ceiling to cycles. Console may print a gem5 command; it does not execute it.

`printLevel > 0` prints detailed array reports; `viewMatStats` adds SI statistics.
At `printLevel <= 0`, cache reports still show each array's sensing metadata, without SI statistics.

### Refresh and result breakdown

Refresh power uses bank refresh energy / saved-cell adjusted retention time.
See [report interpretation](../functionality.md#interpreting-the-report) for
cache totals and refresh contributions to leakage.

Reports read `dramTiming`, `gcDramPower`, `m3d`, and AOS operating points;
see [field units and when they are set](hierarchy.md#structured-result-fields).

Return to the [Programmer's Guide overview](index.md).
