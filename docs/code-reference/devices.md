# Configuration and devices

These classes load configuration and device data. None inherits `FunctionUnit`.
For file syntax, see [Configuration parameters](../configuration.md).

## `InputParameter`

Stores configuration, cell filenames, search bounds, and repeater caches.
`main` owns global `inputParameter`; circuits read it, and `nvsim` rereads it
after tag exploration. Parsing needs no technology or cell.

Sources: [InputParameter.h](https://github.com/neurosim/NS-Cache/blob/main/src/InputParameter.h),
[InputParameter.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/InputParameter.cpp).

```cpp
void ReadInputParameterFromFile(const std::string &inputFile);
void PrintInputParameter();
```

| Method | Use and output |
| --- | --- |
| `ReadInputParameterFromFile` | Requires constructor defaults and a readable path; the application first calls `RESTORE_SEARCH_SIZE`. Updates recognized fields, converts units, then calls private `ValidateInputParameter`. |
| `PrintInputParameter` | Prints current configuration to `cout`; does not evaluate or validate a candidate. |

!!! note "Rereading a file"
    Omitted fields keep their previous values. Cell filenames append to `fileMemCell`.

!!! warning "Parser checks"
    Missing files and checked malformed values exit the process. Aspect-ratio,
    forced-mat, residual-ratio, and tier-limit settings have explicit checks.
    Legacy parsing may ignore unknown keys or accept unsupported combinations;
    `applyConstraint` and candidate initialization perform further checks.

| Fields | Stored units |
| --- | --- |
| `capacity` | bytes |
| `wordWidth`, `pageSize`, `flashBlockSize` | bits |
| `processNode`, `temperature`, `clockFreq` | nm, K, Hz |
| `maxNmosSize` | feature-size multiplier |
| Relative metric constraints | fractional overhead; `Result` uses absolute bounds |

## `MemCell`

Stores cell geometry, electrical data, and three optional AOS records.
`main` owns `sweepCells`; global `cell` selects one. `Mat` and peripherals read
it; `Result::cellTech` borrows it. Methods below use global `tech` and
`inputParameter`; `CalculateReadPower` also reads global `cell`.

Sources: [MemCell.h](https://github.com/neurosim/NS-Cache/blob/main/src/MemCell.h),
[MemCell.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/MemCell.cpp).

```cpp
void ReadCellFromFile(const std::string &inputFile);
void ApplyPVT();
void CellScaling(int _targetProcessNode);
double GetMemristance(double _relativeReadVoltage);
void CalculateWriteEnergy();
double CalculateReadPower();
void PrintCell(int indent = 0);
```

| Method | Inputs, output, and checks |
| --- | --- |
| `ReadCellFromFile` | Initialize configuration/technology first (`vdd` tokens and AOS checks need them). Loads type, geometry, electrical data, retention, and AOS records. Missing files or invalid AOS data exit; legacy validation is incomplete. |
| `ApplyPVT` | Adjusts `retentionTime` (s) from reference/operating temperatures and optional AOS settings; prints the adjustment. Applies to eDRAM/gcDRAM with supplied retention; an unspecified marker returns immediately. AOS errors or nonpositive/nonfinite retention exit. |
| `CellScaling` | Target node: nm, positive (unchecked). Changes selected PCRAM/MRAM resistance, drive, and diode capacitance fields when the old node is positive and differs; updates `processNode`. Other branches are incomplete. Its `main` call is disabled. |
| `GetMemristance` | Input: fraction of read voltage. Returns memristor low-state resistance (Ω), without storing it. Needs nonzero voltage/current anchors and positive resistances (not fully checked). Other cell types warn and return `-1`. |
| `CalculateWriteEnergy` | Uses parsed pulses/drives and `tech` supply fallback. Fills zero `setEnergy`/`resetEnergy` fields (J); preserves supplied nonzero values. Called by relevant `Mat::CalculatePower` branches. No general numeric checks. |
| `CalculateReadPower` | Requires initialized `tech` and global `cell` matching this object. Returns inferred per-path power (W) only when `readPower == 0` and sensing is supported; otherwise `-1`. **Does not write `readPower`.** |
| `PrintCell` | Prints parsed properties at `indent`; no evaluation. |

!!! warning "Call `ApplyPVT` once after parsing"
    Repeated calls compound the retention adjustment.

Stored geometry uses feature-size units (`area` in F²; widths/heights in F).
Parsed pulse/retention times use s, energy J, current A, and power W.

`OxideTransistorParameterSet` holds supplied-field flags, wordline boost/hold
voltages (V), `parameters`, and a validated `operatingPoint`. The parser checks
supply/temperature and caches these before exploration: `oxideAccessTransistor`
for eDRAM; `oxideReadTransistor`/`oxideWriteTransistor` for gcDRAM.
See [AOS devices](../models/aos.md).

## `Technology`

Stores transistor tables, design rules, supply data, and TSV tables.
`main` initializes `tech` and separate eDRAM `devtech`; arrays, wires, and
peripherals read them. Device tables are compiled in; configuration is passed
explicitly.

Sources: [Technology.h](https://github.com/neurosim/NS-Cache/blob/main/src/Technology.h),
[Technology.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/Technology.cpp).

```cpp
void Initialize(int _featureSizeInNano, DeviceRoadmap _deviceRoadmap,
                InputParameter *inputParameter);
void InterpolateWith(Technology rhs, double _alpha);
void SetLayerCount(InputParameter *inputParameter, int layers);
TSV_type WireTypeToTSVType(int wiretype);
void PrintProperty();
```

| Method | Inputs, output, and checks |
| --- | --- |
| `Initialize` | Requires supported node/roadmap and non-null configuration. Stores node (nm/m), device/TSV data, and `initialized`. Unsupported roadmap branches exit; reinitialization warns and proceeds. Application interpolation uses `initializeTechnology`. |
| `InterpolateWith` | Requires initialized objects and a valid blend fraction, normally 0–1 (not clamped). Updates selected electrical/geometry fields only. `rhs` is copied; equal node sizes do nothing. |
| `SetLayerCount` | Requires initialized tables, configuration, and positive die count (unchecked). Updates TSV R/C/area, projection choices, and `layerCount`. Same count returns immediately, even after projection changes. Existing `TSV` objects are unchanged. |
| `WireTypeToTSVType` | Returns `Fine` for aggressive types, `Coarse` for conservative types/`dram_wordline`; unknown values return `Fine`. No state change. |
| `PrintProperty` | **Unimplemented report:** prints a heading and `TO-DO`. |

!!! warning "Temperature index"
    Current tables use `temperature - 300`. Callers must stay within integer
    temperatures 300–400 K. Available entries do not establish calibration;
    see [Transistor technology](../models/transistor-technology.md).

### Vertical-interconnect helper functions

```cpp
double tsv_calc_depwidth(double interface_potential, double t_radius);
double tsv_resistance(double resistivity, double tsv_len, double tsv_diam,
                      double tsv_contact_resistance);
double tsv_capacitance(double tsv_len, double tsv_diam, double tsv_pitch,
                       double dielec_thickness, double liner_dielectric_constant,
                       double depletion_width);
double tsv_area(double tsv_pitch);
```

| Helper | Input units → return units |
| --- | --- |
| `tsv_calc_depwidth` | Potential V, radius m → depletion width m; uses fixed constants. |
| `tsv_resistance` | Resistivity Ω·µm, length/diameter µm, contact resistance Ω → Ω. |
| `tsv_capacitance` | Dimensions µm, relative permittivity dimensionless → F. Replaces supplied `depletion_width` locally using `vdd`. |
| `tsv_area` | Pitch µm → footprint µm². |

These return values without changing state. Positive dimensions and pitch >
diameter are caller requirements; numeric domains are unchecked.
`SetLayerCount` stores results; `TSV::Initialize` converts area to m².

## `AOSFETCompactModel`

Evaluates an AOS device from owned `Parameters` and cached coefficients.
No global dependencies. `MemCell` uses it for validation/retention; `Mat`
reads cached operating points.

Sources: [AOSFETCompactModel.h](https://github.com/neurosim/NS-Cache/blob/main/src/AOSFETCompactModel.h),
[AOSFETCompactModel.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/AOSFETCompactModel.cpp).

```cpp
AOSFETCompactModel();
explicit AOSFETCompactModel(const Parameters &_parameters);
void Initialize(const Parameters &_parameters);
static Parameters DefaultParameters();
double Mobility(double _vgs) const;
double SurfacePotential(double _vgs, double _vch,
                        int _maxIteration = 40, double _tolerance = 1e-8) const;
double CalculateDriftCurrent(double _vgs, double _vds) const;
double CalculateLeakageCurrent(double _vgs, double _vds) const;
double CalculateDrainCurrent(double _vgs, double _vds) const;
double CalculateDrainCurrentExternal(double _vgs, double _vds) const;
double TotalContactResistanceOhm() const;
Charges CalculateCharges(double _vgs, double _vds) const;
Capacitances CalculateCapacitances(double _vgs, double _vds,
                                  double _delta = 1e-3) const;
AOSDeviceOperatingPoint EvaluateOperatingPoint(double _vgsOn,
                                               double _vgsOff, double _vds) const;
```

Constructors call `Initialize`, using `DefaultParameters()` when omitted.
`Initialize` copies/validates parameters and rebuilds coefficients.
`DefaultParameters` returns starting values, not a calibrated device.

!!! note "Before calling"
    Call `Initialize` after editing public `parameters` or after failed
    initialization. Queries are `const`: they return results without storing
    metrics. Voltage inputs must be finite and use V.

| Query | Requirements and return |
| --- | --- |
| `Mobility` | Mobility m²/(V·s), including scale; rejects nonpositive/nonfinite output. |
| `SurfacePotential` | Positive iteration limit/tolerance; tolerance checks normalized solver correction. Returns V; nonconvergence throws. |
| `CalculateDriftCurrent`, `CalculateLeakageCurrent` | Nonnegative `_vds`; intrinsic current A. Drift clamps to zero. |
| `CalculateDrainCurrent` | Intrinsic current A: leakage below flat band, drift otherwise; selected branch checks apply. |
| `CalculateDrainCurrentExternal` | Positive `_vds`; contact-adjusted current A. Zero contact resistance returns intrinsic current; failed solve throws. |
| `TotalContactResistanceOhm` | Initialized parameters; converts width-normalized kΩ·µm to Ω. Zero input returns zero. |
| `CalculateCharges` | Nonnegative `_vds`; `Charges { gate, drain }` in C. Zeros below flat band or for negligible charge sum. |
| `CalculateCapacitances` | Nonnegative `_vds`, finite positive `_delta` (V); `Capacitances { gateSource, gateDrain }` in F, including overlap. Rejects negative/nonfinite output. |
| `EvaluateOperatingPoint` | `_vgsOn > _vgsOff`, positive `_vds`; returns fields below. Enforces `Ion > Ioff > 0`, positive resistance, nonnegative capacitance. |

!!! warning "Exceptions"
    Invalid inputs throw `std::invalid_argument`; singularities, failed solves,
    or invalid computed values throw `std::runtime_error`. Internal exceptions
    propagate. `MemCell` catches these on parsing/retention paths and exits.

`Parameters` uses SI units and K, except VRH/percolation mobility prefactors
(cm²/(V·s)) and contact resistance (kΩ·µm). Constant mobility uses m²/(V·s).
Initialization checks finite values, required signs, and temperature below tail
temperature. Cell parsing additionally checks required fields and temperature
consistency.

| `AOSDeviceOperatingPoint` | Output |
| --- | --- |
| `Ion`, `Ioff` | External on/off drain current, A |
| `Ron`, `Roff` | Effective resistance at each bias, Ω |
| `Cgate` | On-bias gate-source + gate-drain capacitance, F |
| `Cdrain` | On-bias gate-drain capacitance, F |

Next: [Array hierarchy](hierarchy.md).
