# Reading NS-Cache output

Use this guide to locate measurements in `raw.log`. Perform the calculations and comparisons assigned by your instructor, showing your units and assumptions.

First confirm a positive `numSolutions` count and a `RESULT` block. Then check `DESIGN SPECIFICATION`, the selected cell file, and `CONFIGURATION` against the saved input. A completed process or `Finished!` message can also occur when no valid solution was found.

## Organization and units

The hierarchy is **Bank → SubArray → Mat → Cell**. Under `CONFIGURATION`, `Bank Organization` gives subarrays per bank, and `SubArray Organization` gives mats per subarray. Each has `Row Activation` and `Column Activation` counts. `Mat Size` gives cell rows and columns. `Mux Level` lists `Senseamp Mux`, `Output Level-1 Mux`, and `Output Level-2 Mux`.

Read the unit on every value: the display changes prefixes with magnitude. `um` means micrometres; `ps` and `ns` are time; `fJ`, `pJ`, and `nJ` are energy; `nW`, `uW`, and `mW` are power. Area may be printed in `um^2` or `mm^2`. Convert dimensions before multiplying, and square the length-conversion factor when converting area.

Input capacities use binary units (`KB` = KiB, `MB` = MiB). Word width is in **bits**. Reported bandwidth is in **bytes per second**, with decimal prefixes such as `GB/s`. The `Power` section contains both dynamic **energy per access** and leakage **power**; these are different quantities.

## Fields for each question

In the table, a path such as `Timing → Read Latency → Mat Latency` means the indented entry within that read branch. Several labels occur again under the write branch.

| Question | Locate these fields |
| --- | --- |
| Q1 | `CONFIGURATION` organization, activation, mat size, and mux entries. Under `Timing → Read Latency`, use `Row Decoder Latency` and `Bitline Latency` within `Mat Latency`, and the top-level `Read Latency` for the complete bank access. |
| Q2 | The same configuration entries; `Power → Read Dynamic Energy → SubArray Dynamic Energy → Mat Dynamic Energy → Senseamp Dynamic Energy`; top-level `Read Dynamic Energy`; and read-branch `Bitline Latency`, `Mux Latency`, and total `Read Latency`. |
| Q3 | Under `Timing → Read Latency`: `H-Tree Latency`, `SubArray Latency → Predecoder Latency`, and `SubArray Latency → Mat Latency`. Use top-level `Read Latency` for the total. Under `Power → Read Dynamic Energy`, use that total and its `H-Tree Dynamic Energy` child. |
| Q4 | Read-branch `Mat Latency`, `Row Decoder Latency`, and `Precharge Latency`; **write-branch** `Mat Latency`; top-level bank `Read Latency`; `Read Bandwidth`, `Write Bandwidth`; and `Area → Total Area`. Reuse Q3 logs. |
| Q5 | `Area → Total Area`; `Timing → Read Bandwidth` and `Write Bandwidth`; and `Power → Leakage Power`. |
| Q6 | From the MRAM Q5 log, `Timing → Write Latency → SubArray Latency → Mat Latency` and its `Write Pulse Duration`. Use reported bank areas and the SRAM mat write latency from Q5 for the requested calculation. |
| Q7 | Use the assigned cache parameters, routing table, and scheduling diagram. No simulator output is needed. |
| Q8 | Top-level `Power → Read Dynamic Energy`, its `H-Tree Dynamic Energy` child, and `Timing → Read Bandwidth`. Check word width in `DESIGN SPECIFICATION` and organization/mux settings in `CONFIGURATION`. |
| Q9 | Before `SUMMARY`, the passive-wire report gives `Wire Resistance` in `ohm/um` and `Wire Capacitance` in `fF/um`. The active-wire report gives `Repeater Spacing` in `mm` and `Delay` in `ns/mm`. This `Delay` is repeated-wire delay **per length**, not bank access time. |
| Q10 | From the Q9 logs: top-level bank `Timing → Read Latency` and its `H-Tree Latency` child provide the bank and H-tree delays used to define the speedup parameters. |
| Q11 | Use circuit functions and the course assignment. No additional run is required. |

## Interpretation notes

- **Parent and child values overlap.** A bank delay contains its subarray contribution, and a subarray delay contains its mat contribution. Do not add a parent to its children. The printed `Mux Latency` combines the sense-amp input mux and both output-mux stages.
- **Keep the energy scope.** `Mat Dynamic Energy` is per active mat; `SubArray Dynamic Energy` is per subarray. The top-level bank energy already includes the active instances. `Senseamp Dynamic Energy` describes the sensing circuitry of one mat. The baseline and transfer experiments activate different numbers of subarrays.
- **Bandwidth has a model-specific cycle.** Read bandwidth uses the mat-cycle definition in the course assignment. It is not the reciprocal of total bank read latency and is not a workload-throughput prediction.
- **Area is modeled layout area.** The efficiency printed beside `Mat Area` uses nominal cell area from the cell file. The current SRAM layout can omit sense-amplifier area; treat residual area as modeled overhead and do not infer physical sense-amplifier area savings. The MRAM input is an illustrative cell assumption with the selected CMOS peripherals.
- **Displayed values are rounded.** Expect small differences when reconstructing quantities from printed components. Some tiny wire-level energy or leakage values can round to zero.

The supplied configurations use the normal report. Leave their fixed settings unchanged; extra diagnostic and clock-quantization output is not required for these questions.
