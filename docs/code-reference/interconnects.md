# Interconnects

`Wire` evaluates a wire length; `TSV` evaluates one vertical connection.
Callers scale their outputs by routed bits, active paths, or connection counts.

## `Wire`

Stores geometry, RC, repeaters, and an owned low-swing `SenseAmp` pointer.
`nvsim` owns global `localWire`/`globalWire`; each `Result` owns saved copies.
Banks evaluate lengths; mats/predecoders read unit RC. Requires global `tech`
and `inputParameter`; low swing also uses `cell`.

Sources: [Wire.h](https://github.com/neurosim/NS-Cache/blob/main/src/Wire.h),
[Wire.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Wire.cpp).

### Initialization and length evaluation

```cpp
void Initialize(int _featureSizeInNano, WireType _wireType,
                WireRepeaterType _wireRepeaterType,
                int _temperature, bool _isLowSwing);
void CalculateLatencyAndPower(double _wireLength, double *delay,
                              double *dynamicEnergy, double *leakagePower);
```

| Method | Inputs and output |
| --- | --- |
| `Initialize` | Requires initialized globals. Inputs: node nm, wire/repeater enums, temperature K, low-swing flag. Stores geometry m, resistance Ω/m, capacitance F/m, repeater size/spacing/layout, and `initialized`. Selects repeaters internally. Reinitialization and destruction delete the current receiver. |
| `CalculateLatencyAndPower` | Requires initialization and valid physical length (m)/device state. Writes delay s, energy J, leakage W through distinct output pointers. Passive full-swing leakage is zero; repeated-wire outputs scale unit values by length. |

!!! warning "Call requirements and checks"
    Full-swing outputs may be null; low swing requires all three pointers.
    Length/device inputs are not generally checked. An uninitialized call prints
    an error without establishing outputs. Low swing with repeaters exits.

!!! warning "Low-swing implementation"
    Evaluates transmitter/wire contributions and allocates a receiver. It
    prepares receiver RC, but reads receiver latency/power without evaluating
    them. Repeated calls replace the receiver pointer without deleting the
    previous receiver.

### Repeater selection and unit queries

```cpp
void findOptimalRepeater();
void findPenalizedRepeater(double _penalty);
double getRepeatedWireUnitDelay();
double getRepeatedWireUnitDynamicEnergy();
double getRepeatedWireUnitLeakage();
void PrintProperty();
Wire &operator=(const Wire &rhs);
```

| Method | Requirements and output |
| --- | --- |
| `findOptimalRepeater` | Prepared wire RC/device data. Writes `repeaterSize` (minimum-driver multiplier), `repeaterSpacing` (m). Called before `initialized` is set; no guard/status return. |
| `findPenalizedRepeater` | Existing size/spacing. `_penalty`: allowed fractional delay overhead, e.g. `0.10` (unchecked). Selects lower-energy/leakage size/spacing under that bound. |
| `getRepeatedWireUnitDelay` | Returns s/m. |
| `getRepeatedWireUnitDynamicEnergy` | Returns J/m per switching event. |
| `getRepeatedWireUnitLeakage` | Returns W/m. |
| `PrintProperty` | Prints passive RC or repeater settings/unit metrics to `cout`; requires matching initialized state. |
| `operator=` | Returns `*this`; copies selected geometry/repeater/RC fields, excluding receiver pointer and M0/M1 resistance fields. Use `Initialize` to rebuild a trial wire. |

Unit queries require positive repeater size/spacing and prepared RC; no checks
or metric writes. Repeater selection does not refresh layout; `Initialize`
calculates pitch afterward. These are separate from mat wordline repeaters.

## `TSV`

Models an optionally buffered vertical connection, including monolithic vias.
Inherits `FunctionUnit`; owns fixed buffer arrays/scalars. Banks, subarrays,
and mats own it by value. Uses global `tech` tables and `inputParameter`
temperature.

Sources: [TSV.h](https://github.com/neurosim/NS-Cache/blob/main/src/TSV.h),
[TSV.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/TSV.cpp).

```cpp
void Initialize(TSV_type requestedType, bool buffered = false);
void CalculateArea();
void CalculateLatencyAndPower(double _rampInputRead, double _rampInputWrite);
TSV &operator=(const TSV &rhs);
```

| Method | Requirements, state written, and checks |
| --- | --- |
| `Initialize` | First call `Technology::SetLayerCount` with the intended layer count. Takes `Fine`, `Coarse`, or `Monolithic` enum (bounds unchecked). Copies `res` Ω, `cap` F, `min_area` m² (from µm²). Later table changes do not update these copies. Optionally sizes buffers; default `num_gates == 0`. Sets flags; stage limit can mark invalid, buffered sizing also asserts. |
| `CalculateArea` | Successful initialization required; no invalid/uninitialized guard. Writes metal/buffer/total `area`, square `width`/`height`, and buffer `leakage`. |
| `CalculateLatencyAndPower` | Initialized state; asserts nonzero read/write input slopes. Writes latency/energy per connection; set/reset copy write fields. Does not update leakage or scale by count. |
| `operator=` | Copies TSV state, buffer arrays, counts, and metrics; returns `*this`. No evaluation. |

!!! warning "Energy accumulates without buffers"
    Unbuffered evaluation adds to read/write energy. Neither this call nor
    `Initialize` resets it. Use a fresh object for a single-evaluation value.

| Fields | Meaning |
| --- | --- |
| `C_load_TSV` | Buffered load, F |
| `w_TSV_n`, `w_TSV_p` | Buffer widths, m |
| `numTotalBits`, `numAccessBits`, `numReadBits`, `numDataBits` | Owner-set accounting counts; owners scale area/energy/leakage/latency as needed. |

For monolithic folding and stacked-die routing, see
[Monolithic 3D](../models/monolithic-3d.md).

Next: [Results and reporting](results.md).
