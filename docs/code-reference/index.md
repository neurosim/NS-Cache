# Programmer's Guide

Use this reference to find a function's inputs, call order, outputs, and errors.
It covers the simulator in `src/`.

!!! note "Scope"
    `formula.cpp` functions and equations will be documented later. gem5 internals are excluded.
    For model explanations, see [Architecture](../architecture.md) and
    [Functionality and results](../functionality.md).

## Example: Trace a supplied SRAM configuration

Run from the repository root:

```sh
make -C src -j2
./nsc config/New_Configs/SRAM_cache_14nm.cfg
```

The [14 nm SRAM example](../getting-started.md#sram-cache-example) selects a
32 MiB, 16-way cache: 512-bit lines, normal access, H-tree, voltage sensing,
and read-latency optimization. Data-bank bounds are 4 × 4 subarrays; other
choices are searched. Tags use reduced bounds.

```text
main
  read configuration → initializeTechnology(tech)
  load every cell → MemCell::ApplyPVT()
  set cell, devtech = tech → construct data/tag Result arrays
  nvsim
    allocate local/global wires
    search tags → refine wires → restore configuration
    search data → refine wires → apply any relative limits
    [inside search loops: save valid candidates with compareAndUpdate]
  choose objective winner → Result::printAsCache
```

Each candidate follows this call order:

```text
CALCULATE(bank, memoryType)
  allocate BankWithHtree or BankWithoutHtree
  bank.Initialize(...)
    subarray.Initialize(...)
      mat.Initialize(...)       // Cell loads and peripheral RC
      mat.CalculateArea()       // Layout needed by predecoders
      initialize predecoders / tag comparator
    [non-H-tree: area and external-sensing setup]
  bank.CalculateArea()
  bank.CalculateRC()            // Predecoders and global circuits
  bank.CalculateLatencyAndPower()
    subarray.CalculateLatency(infinite_ramp)
      predecoder → mat → optional tag comparison
    subarray.CalculatePower()
      predecoders → mat → optional tag comparator → scale counts
    add bank routing / TSV terms; check validity
  check bank.invalid → retain or reject candidate
```

## Functions and units

Most `Initialize` and `Calculate*` methods return `void`; read their outputs
from object fields. `CalculatePower()` writes dynamic **energy** and leakage
**power**.

| Quantity | Stored unit |
| --- | --- |
| Dimensions / area | m / m² |
| Latency / energy / leakage | s / J / W |
| Resistance / capacitance / voltage / current | ohm / F / V / A |
| Capacity | Configuration: bytes. Bank API: bits. |
| Word, block, data widths | bits |
| Cell geometry | Feature-size multiples, converted to m by circuit code |
| Process / temperature | nm / K |
| Timing ramp | Inverse-time slope; `infinite_ramp` is an ideal edge |

The cell parser converts ns, pJ, µW, µA, and µs to SI. Technology TSV tables
use µm and µm². Function descriptions list other exceptions.

## Errors

!!! warning "Check validity before reading metrics"
    `initialized` means setup ran; `invalid` can still be true.
    `invalid_value` is the finite marker `1e41`. Initial zero fields do not
    mean a circuit has zero cost.

| Failure path | Behavior |
| --- | --- |
| Calculation before initialization | Usually prints an error; may leave old/default fields. |
| Invalid candidate | Skips work or writes markers. |
| Parsing / unsupported mode | May terminate the process. |
| Standalone AOS validation | Throws exceptions. |

Keep the [global context](https://github.com/neurosim/NS-Cache/blob/main/src/global.h)
(configuration, technology, cell, wires) alive and consistent. The interfaces
are not thread-safe. See [Array hierarchy](hierarchy.md#evaluation-and-copying)
for copying saved results.

Next: [Execution and lifecycle](execution.md).
