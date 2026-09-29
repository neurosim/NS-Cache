# Execution and lifecycle

`main` loads the inputs. `nvsim` searches one cell type and saves the best designs.

## Basic usage

```text
./nsc [configuration.cfg]
```

The default configuration is `nvsim.cfg`. Relative cell-file paths use the
working directory. See [Running NS-Cache](../getting-started.md#compilation)
for the build command and [the SRAM example](index.md#example-trace-a-supplied-sram-configuration)
for a complete run.

!!! note "Output"
    Selected-objective runs print the best result. Full exploration writes a
    CSV beside the configuration, replacing an existing file with that name.

Sources: [main.cpp](https://github.com/neurosim/NS-Cache/blob/main/src/main.cpp),
[macros.h](https://github.com/neurosim/NS-Cache/blob/main/src/macros.h),
[global.h](https://github.com/neurosim/NS-Cache/blob/main/src/global.h).

## `main`

```cpp
int main(int argc, char *argv[]);
```

1. Create `inputParameter`, read configuration, and initialize `tech`.
2. Load every cell into `sweepCells` and call `MemCell::ApplyPVT()` once per cell.
3. Set global `cell`; set `devtech = tech`, except eDRAM uses a separate EDRAM table.
4. Allocate data/tag `Result` arrays and call `nvsim` for each cell.
5. Print the winner or full-exploration output. `allowDifferentTagTech` controls
   whether data and tag winners must use the same cell.

**Lifetime:** configuration, cells, and saved cell pointers must survive reporting.
**Return:** normally `0`, including no usable data solution; check solution counts.
CSV-open failure, input errors, and unsupported settings can terminate the process early.

## `initializeTechnology`

```cpp
void initializeTechnology(Technology *target, int processNode,
                          DeviceRoadmap deviceRoadmap);
```

| Input / output | Details |
| --- | --- |
| `target` | Non-null, writable technology object; caller owns it. |
| `processNode` | Process size in nm. |
| `deviceRoadmap` | Device table to load. |
| Global input | Initialized `inputParameter`. |
| Writes | `target` fields: calls `Technology::Initialize`, then interpolates between tables for supported node intervals. |
| Failure | Unsupported combinations can terminate in `Technology::Initialize`; no status is returned. |

## `nvsim`

```cpp
int nvsim(ofstream& outputFile, string inputFileName,
          long long& numSolution, Result *bestDataResults,
          Result *bestTagResults);
```

!!! note "Before calling"
    Parse configuration and set `cell`, `tech`, and `devtech`. Construct both
    result arrays with `(int)full_exploration` entries. Set `numSolution = 0`.
    Open `outputFile` for CSV mode. Keep `inputFileName` readable.

Execution order:

```text
applyConstraint → allocate localWire/globalWire
  cache: reduce bounds → search tags → refine tag wires
         restore defaults → reread configuration → applyConstraint
  set data width and associativity → search data → refine data wires
  relative constraints: set absolute limits → repeat data search
  delete global wire objects
```

Capacity is converted from bytes to bits before bank initialization.

| Mode | Data-search setup |
| --- | --- |
| Sequential cache | One effective way. |
| Fast cache | Access a full set; one effective way. |
| Normal cache | One row per set. |
| DRAM / SLC NAND RAM | Access width is `pageSize`. |

Tag width includes valid/dirty bits and any partition padding.

**Writes:** both result arrays, objective IDs, saved cell pointers, search
bounds, global wires, and `numSolution`. Prints attempted/accepted counts.
`numSolution` resets between tag/data searches and for constrained data search;
it excludes a combined tag/data total and wire-refinement trials.

**Return:** `1` if no valid cache tag exists; otherwise `0`, even with zero data
solutions. Global wire objects are deleted, but their pointers are not cleared.
Saved results own separate wire objects.

!!! warning "Unfinished paths"
    Full-exploration pruning allocates limits and prints results, but its
    pruning pass is still `TO-DO`. Cross-cell `doublePrune` is also unfinished.

## `applyConstraint`

```cpp
void applyConstraint();
```

Called by `nvsim`; reads global `inputParameter` and `cell`.

| Condition | Action |
| --- | --- |
| Non-cache associativity > 1 | Set associativity to 1. |
| CAM, MLC NAND, or H-tree with external sensing | Print error and exit. |
| Associativity is not a power of two | Print error; **continue** (exit is commented out). |

Returns `void`. Numeric objective limits are handled later by `nvsim` and `Result`.

## Search and evaluation macros

!!! note "Usage"
    These macros run inside `nvsim` and use its local variables and global settings.
    Use them in the existing search loops.

| Macro | Effect |
| --- | --- |
| `RESTORE_SEARCH_SIZE` | Set built-in organization/wire bounds; parsing then narrows them. |
| `REDUCE_SEARCH_SIZE` | Set reduced tag-search bounds, including buffers and wires. |
| `REDUCE_SEARCH_SIZE_CONSTRAINED` | Reduced bounds retaining fixed dimensions; currently unused. |
| `BIGFOR` | Organization loops; most counts double, buffer targets increment. Counts that double must start above zero. |
| `INITIAL_BASIC_WIRE` | Initialize allocated global wires from forced or baseline settings. |
| `CALCULATE(bank, memoryType)` | Allocate bank; call `Initialize` → `CalculateArea` → `CalculateRC` → `CalculateLatencyAndPower`. Caller checks `invalid` and deletes bank. |
| `VERIFY_DATA_CAPACITY`, `VERIFY_TAG_CAPACITY` | Rebuild capacity from dimensions/counts. Mismatch prints dimensions and calls `exit(-1)`. |
| `UPDATE_BEST_DATA`, `UPDATE_BEST_TAG` | Copy valid bank/wires into `tempResult`; call `compareAndUpdate` for each objective. |
| `REFINE_LOCAL_WIRE_FORLOOP`, `REFINE_GLOBAL_WIRE_FORLOOP` | Enumerate wire/repeater/swing choices; skip low-swing with repeaters. Loop body performs evaluation. |
| `LOAD_LOCAL_WIRE(oldResult)`, `LOAD_GLOBAL_WIRE(oldResult)` | Reinitialize global wire from saved choice and current process/temperature. |
| `TRY_AND_UPDATE(oldResult, memoryType)` | Allocate/evaluate trial using saved organization and current wires; copy, compare, delete. Requires `trialBank` and `tempResult` in scope. |
| `APPLY_LIMIT(result)` | Reset primary metrics; assign enclosing scope's absolute latency/energy/EDP/area/leakage limits. |
| `OUTPUT_TO_FILE` | Write `tempResult`: cache paired with each objective's tag result, or array record plus newline. Requires an open stream. |

`TO_SECOND`, `TO_JOULE`, `TO_WATT`, `TO_METER`, `TO_SQM`, `TO_BPS`, and
`TO_GENERAL` produce stream expressions with display units. They do not change
stored values or return strings.

Next: [Configuration and devices](devices.md).
