/*
 * Binary current-mode sense-amplifier adaptation for NS-Cache.
 * The fitted sensing equations originate in NeuroSim (CC BY-NC 4.0).
 * See BinarySenseAmp.cpp for the retained notice and source provenance.
 */
#ifndef BINARY_SENSE_AMP_H_
#define BINARY_SENSE_AMP_H_

#include <string>
#include "Technology.h"

enum class BinarySenseAmpStatus { Applied, Fallback, Invalid };

/* Explicit values prevent the evaluator from consulting mutable global state.
 * Resistance includes the selected cell's access resistance, in ohms.
 * columnCapacitance is the external column/access load, in farads: it excludes
 * the CSA's own input devices and mux capacitance. referenceColumnArea is the
 * footprint of one full-height matched dummy column, in square metres.
 */
struct BinarySenseAmpOperatingPoint {
    bool contextAvailable = false;
    bool internalSenseAmp = false;
    bool readPowerOverride = false;
    int tierCount = 1;
    MemCellType cellType = MRAM;
    CellAccessType accessType = CMOS_access;
    double temperatureK = 300;
    double readVoltage = 0;
    double resistanceOn = 0;
    double resistanceOff = 0;
    double columnCapacitance = 0;
    double referenceColumnArea = 0;
};

struct BinarySenseAmpResult {
    BinarySenseAmpStatus status = BinarySenseAmpStatus::Fallback;
    std::string reason;
    double referenceResistance = 0;
    double resistanceMargin = 0;
    double nominalReadVoltage = 0;
    double inputCapacitance = 0;  // One data port; the matched reference is separate.
    double gateCapacitanceN = 0;
    double gateCapacitanceP = 0;
    double junctionCapacitanceN = 0;
    double junctionCapacitanceP = 0;
    double readLatency = 0;       // One parallel sensing event, seconds.

    // The following totals include numAmplifiers; do not multiply them again.
    double coreArea = 0;
    double referenceArea = 0;
    double area = 0;
    double columnSwitchingEnergy = 0;  // Selected data + dedicated reference column.
    double internalSwitchingEnergy = 0; // Input/mirror, latch, output interface.
    double operatingEnergy = 0;         // Fitted enabled CSA/cell operating power * delay.
    double readDynamicEnergy = 0;
    double leakage = 0;                 // Analytical standby subthreshold estimate, W.
    double perAmplifierArea = 0;
    double perAmplifierReadDynamicEnergy = 0;
    double perAmplifierLeakage = 0;
};

/* The pure numerical fits are exposed for independent provenance tests.
 * They contain no technology helpers, global state, fanout, or reference-vector
 * indexing. Unsupported nodes and malformed inputs return quiet NaN.
 * Latency receives external column C only. Power preserves upstream's 0.5/Vread
 * resistance normalization and temperature multiplier. Negative extrapolated
 * fit results are intentionally exposed here and rejected by the evaluator.
 */
double BinarySenseAmpNominalReadVoltage(int nodeNm);
double BinarySenseAmpLatencyFit(int nodeNm, double resistance, double columnCapacitance);
double BinarySenseAmpOperatingPowerFit(int nodeNm, double resistance,
                                      double readVoltage, double temperatureK = 300);

/* Applies only within the declared adaptation boundary. Fallback returns zero
 * model outputs and a reason; the caller must evaluate its legacy CSA. Invalid
 * reports a malformed eligible point or a nonphysical numerical result; callers
 * must reject that candidate rather than substitute a different model.
 */
BinarySenseAmpResult EvaluateBinarySenseAmp(const BinarySenseAmpOperatingPoint& point,
                                          const Technology& technology,
                                          long long numAmplifiers);

#endif
