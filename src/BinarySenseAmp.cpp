/*******************************************************************************
* Copyright (c) 2015-2017
* School of Electrical, Computer and Energy Engineering, Arizona State University
* PI: Prof. Shimeng Yu
* All rights reserved.
*
* This source code is part of NeuroSim - a device-circuit-algorithm framework to benchmark
* neuro-inspired architectures with synaptic devices(e.g., SRAM and emerging non-volatile memory).
* Copyright of the model is maintained by the developers, and the model is distributed under
* the terms of the Creative Commons Attribution-NonCommercial 4.0 International Public License
* http://creativecommons.org/licenses/by-nc/4.0/legalcode.
* The source code is free and you can redistribute and/or modify it
* by providing that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Developer list:
*   Pai-Yu Chen     Email: pchen72 at asu dot edu
*   Xiaochen Peng   Email: xpeng15 at asu dot edu
********************************************************************************/

/* NS-Cache binary adaptation, 2026.
 * Source: neurosim/NeuroSim, commit 9825ef40bf14d12a72c99d8e32ff8c499aeddf24,
 * NeuroSIM/MultilevelSenseAmp.cpp: updated current-mode ColumnLatency_Table,
 * LSTP GetColumnPower, switching energy, and layout-equivalent area equations.
 * Manual: User Manual of DNN simulator_V1.5.pdf, section 6.2.7, pp.19-24.
 *
 * This is a semi-empirical binary adaptation, not a SPICE-validated cache CSA.
 * The delay fit represents SAE-to-output sensing with a 10% resistance margin
 * and node-specific fixed read voltage. No certified R/C fit envelope is given
 * upstream. The binary comparator has zero additional-comparator fanout; the
 * original Rref[1] / unsigned size()-2 wrapper cannot safely represent it.
 *
 * One dedicated, ideal adjustable dummy column supplies the conductance-midpoint
 * reference. Its access enable stays asserted; Vread is pulsed, so no dummy-row
 * activation energy is added. Operating power is counted once per comparator:
 * the manual describes the fit as containing main/reference cells and transistors.
 * The caller must remove its separate column switching and cell read energy, and
 * avoid charging this CSA's input capacitance again in upstream mux power.
 * Wire RC and mux propagation remain outside this sensing evaluator.
 */

#include "BinarySenseAmp.h"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <limits>
#include "formula.h"

namespace {

struct NodeFit {
    int node;
    double readVoltage;
    double latencyBreak1, latencyLogSlope1, latencyLogIntercept1;
    double latencyBreak2, latencyLogSlope2, latencyLogIntercept2;
    double latencyLinearSlope, latencyLinearIntercept;
    double referenceCapacitance, capDelay, referenceResistance, capResistanceThreshold;
    double powerBreak1, powerLinearSlope, powerLinearIntercept;
    double powerBreak2, powerLogSlope1, powerLogIntercept1;
    double powerLogSlope2, powerLogIntercept2;
};

// Equal latency breakpoints mean a single logarithmic branch, then linear.
// A zero second power breakpoint means its logarithmic branch has no further split.
const NodeFit kFits[] = {
    {90, .58, 4832, -1.08914e-10, 2.43204e-9, 4832, 0, 0,
     2.8688e-14, 2.31147e-9, 18.4e-15, 1.5e-9, .1e6, 4.83e3,
     4832, -1.67004e-11, 8.54958e-7, 379269, -1.50719e-7, 2.0858e-6,
     -8.89685e-8, 1.3007e-6},
    {65, .55, 4832, -8.11436e-11, 2.2554e-9, 4832, 0, 0,
     2.12366e-14, 2.14339e-9, 13.3e-15, 1.2e-9, .1e6, 4.83e3,
     4832, -6.08647e-12, 4.82814e-7, 379269, -8.03449e-8, 1.16437e-6,
     -6.01171e-8, 8.9355e-7},
    {45, .51, 4832, -1.12976e-10, 2.16762e-9, 4832, 0, 0,
     1.54619e-14, 1.60263e-9, 9.21e-15, .81e-9, .1e6, 4.83e3,
     4832, -3.96471e-12, 3.44344e-7, 379269, -5.66235e-8, 8.27849e-7,
     -4.32673e-8, 6.46111e-7},
    {32, .51, 4832, -1.31777e-10, 2.03151e-9, 4832, 0, 0,
     1.15509e-14, 1.19475e-9, 6.55e-15, .55e-9, .1e6, 4.83e3,
     4832, -6.72783e-12, 4.19847e-7, 379269, -7.28881e-8, 1.02677e-6,
     -4.35333e-8, 6.4697e-7},
    {22, .55, 4832, -1.32118e-10, 1.7552e-9, 4832, 0, 0,
     8.02517e-15, 8.51803e-10, 4.5e-15, .31e-9, .1e6, 12.7e3,
     4832, -2.85373e-11, 9.16744e-7, 379269, -1.63133e-7, 2.1687e-6,
     -5.8712e-8, 8.64648e-7},
    {14, .277, 812, -7.01209e-11, 1.21771e-9, 2962, -2.11644e-11, 8.96216e-10,
     6.18337e-15, 8.17633e-10, 2.86e-15, .29e-9, .1e6, 12.7e3,
     7847, -3.11213e-13, 5.34821e-8, 0, -9.64978e-9, 1.44527e-7, 0, 0},
    {10, .28, 1128, -7.20238e-11, 1.18578e-9, 4832, -1.81368e-11, 8.16065e-10,
     4.72185e-15, 7.28474e-10, 2.04e-15, .27e-9, .1e6, 12.7e3,
     20691, -3.27225e-13, 5.54082e-8, 0, -9.99199e-9, 1.49539e-7, 0, 0},
    {7, .264, 4832, -4.67061e-11, 1.03708e-9, 4832, 0, 0,
     3.72084e-15, 7.02172e-10, 1.43e-15, .13e-9, .1e6, 12.7e3,
     20691, -2.62089e-13, 4.7357e-8, 0, -8.49198e-9, 1.27848e-7, 0, 0},
    {5, .253, 4832, -4.33009e-11, 9.79525e-10, 7847, -3.15954e-12, 6.53005e-10,
     2.77641e-15, 6.46706e-10, 1.02e-15, .09e-9, .1e6, 12.7e3,
     20691, -1.4179e-13, 3.31434e-8, 0, -5.76552e-9, 8.89536e-8, 0, 0},
    {3, .248, 4832, -3.62939e-11, 9.04595e-10, 7847, -8.05167e-12, 6.69134e-10,
     2.02367e-15, 6.04792e-10, .61e-15, .05e-9, .1e6, 12.7e3,
     20691, -1.17664e-13, 2.96368e-8, 0, -5.09202e-9, 7.91852e-8, 0, 0}
};

const NodeFit* FindFit(int node) {
    for (const auto& fit : kFits) {
        if (fit.node == node) return &fit;
    }
    return nullptr;
}

double NaN() { return std::numeric_limits<double>::quiet_NaN(); }
bool Positive(double value) { return std::isfinite(value) && value > 0; }
bool Nonnegative(double value) { return std::isfinite(value) && value >= 0; }
bool AllNonnegative(std::initializer_list<double> values) {
    for (double value : values) if (!Nonnegative(value)) return false;
    return true;
}

BinarySenseAmpResult Reject(BinarySenseAmpStatus status, const char* reason) {
    BinarySenseAmpResult result;
    result.status = status;
    result.reason = reason;
    return result;
}

}  // namespace

double BinarySenseAmpNominalReadVoltage(int nodeNm) {
    const NodeFit* fit = FindFit(nodeNm);
    return fit ? fit->readVoltage : NaN();
}

double BinarySenseAmpLatencyFit(int nodeNm, double resistance, double columnCapacitance) {
    const NodeFit* fit = FindFit(nodeNm);
    if (!fit || !Positive(resistance) || !Nonnegative(columnCapacitance)) return NaN();
    double latency;
    if (resistance < fit->latencyBreak1) {
        latency = fit->latencyLogSlope1 * std::log(resistance) + fit->latencyLogIntercept1;
    } else if (resistance < fit->latencyBreak2) {
        latency = fit->latencyLogSlope2 * std::log(resistance) + fit->latencyLogIntercept2;
    } else {
        latency = fit->latencyLinearSlope * resistance + fit->latencyLinearIntercept;
    }
    if (resistance > fit->capResistanceThreshold) {
        latency += fit->capDelay / (fit->referenceResistance - fit->capResistanceThreshold)
            * (resistance - fit->capResistanceThreshold)
            * ((columnCapacitance - fit->referenceCapacitance) / fit->referenceCapacitance);
    }
    return latency;
}

double BinarySenseAmpOperatingPowerFit(int nodeNm, double resistance,
                                      double readVoltage, double temperatureK) {
    const NodeFit* fit = FindFit(nodeNm);
    if (!fit || !Positive(resistance) || !Positive(readVoltage) || !Positive(temperatureK)) return NaN();
    resistance *= 0.5 / readVoltage;
    if (!Positive(resistance)) return NaN();
    double power;
    if (resistance < fit->powerBreak1) {
        power = fit->powerLinearSlope * resistance + fit->powerLinearIntercept;
    } else if (fit->powerBreak2 == 0 || resistance < fit->powerBreak2) {
        power = fit->powerLogSlope1 * std::log(resistance) + fit->powerLogIntercept1;
    } else {
        power = fit->powerLogSlope2 * std::log(resistance) + fit->powerLogIntercept2;
    }
    return power * (1 + 1.3e-3 * (temperatureK - 300));
}

BinarySenseAmpResult EvaluateBinarySenseAmp(const BinarySenseAmpOperatingPoint& point,
                                          const Technology& technology,
                                          long long numAmplifiers) {
    using Status = BinarySenseAmpStatus;
    if (!point.contextAvailable) return Reject(Status::Fallback, "no-operating-point");
    if (!point.internalSenseAmp) return Reject(Status::Fallback, "external-sensing");
    // Once a concrete internal operating point is supplied, malformed values
    // are errors even if another field would otherwise request legacy fallback.
    if (numAmplifiers <= 0 || !Positive(point.resistanceOn) || !Positive(point.resistanceOff)
            || point.resistanceOff < point.resistanceOn || !Positive(point.columnCapacitance)
            || !Positive(point.referenceColumnArea)) return Reject(Status::Invalid, "invalid-operating-point");
    if (!Positive(point.temperatureK) || !Nonnegative(point.readVoltage))
        return Reject(Status::Invalid, "invalid-read-bias-or-temperature");
    if (point.tierCount != 1) return Reject(Status::Fallback, "multiple-memory-tiers");
    if (point.cellType != MRAM && point.cellType != PCRAM && point.cellType != memristor)
        return Reject(Status::Fallback, "unsupported-cell-type");
    if (point.accessType != CMOS_access) return Reject(Status::Fallback, "unsupported-access-type");
    if (point.readPowerOverride) return Reject(Status::Fallback, "explicit-cell-read-power");
    if (!technology.initialized) return Reject(Status::Invalid, "uninitialized-technology");
    if (technology.deviceRoadmap != LSTP) return Reject(Status::Fallback, "unsupported-roadmap");
    const NodeFit* fit = FindFit(technology.featureSizeInNano);
    if (!fit) return Reject(Status::Fallback, "unsupported-node");
    if (!Positive(technology.featureSize) || !Positive(technology.vdd)
            || !Positive(technology.pnSizeRatio)) return Reject(Status::Invalid, "invalid-technology");
    if (technology.featureSize != fit->node * 1e-9)
        return Reject(Status::Fallback, "interpolated-node");
    if (point.temperatureK != 300)
        return Reject(Status::Fallback, "unsupported-temperature");
    if (std::abs(point.readVoltage - fit->readVoltage) > 1e-12)
        return Reject(Status::Fallback, "non-nominal-read-voltage");
    BinarySenseAmpResult result;
    result.nominalReadVoltage = fit->readVoltage;
    // Stable harmonic mean; no resistance product that could overflow.
    result.referenceResistance = point.resistanceOn / (.5 + .5 * point.resistanceOn / point.resistanceOff);
    // For the harmonic reference, the on-side normalized separation is the
    // smaller of |Rstate-Rref|/max(Rstate,Rref). Its reduced expression avoids
    // cancellation in Rref-Ron and a rounding-dependent 10% boundary.
    result.resistanceMargin = ((point.resistanceOff - point.resistanceOn) / point.resistanceOff) * .5;
    if (!Positive(result.referenceResistance) || !Nonnegative(result.resistanceMargin))
        return Reject(Status::Invalid, "invalid-reference-resistance");
    if (result.resistanceMargin < .10)
        return Reject(Status::Fallback, "insufficient-resistance-margin");

    const double widthN = MIN_NMOS_SIZE * technology.featureSize;
    const double widthP = technology.pnSizeRatio * widthN;
    const double height = MAX_TRANSISTOR_HEIGHT * technology.featureSize;
    CalculateGateCapacitance(INV, 1, widthN, 0, height, technology,
        &result.gateCapacitanceN, &result.junctionCapacitanceN);
    CalculateGateCapacitance(INV, 1, 0, widthP, height, technology,
        &result.gateCapacitanceP, &result.junctionCapacitanceP);
    if (!AllNonnegative({result.gateCapacitanceN, result.gateCapacitanceP,
            result.junctionCapacitanceN, result.junctionCapacitanceP}))
        return Reject(Status::Invalid, "invalid-device-capacitance");
    result.inputCapacitance = 2 * result.gateCapacitanceN + result.junctionCapacitanceN;
    double transistorHeight = 0, transistorWidth = 0;
    CalculateGateArea(INV, 1, widthN, 0, height, technology, &transistorHeight, &transistorWidth);
    const double coreArea = 9 * transistorHeight * transistorWidth;
    result.readLatency = BinarySenseAmpLatencyFit(fit->node, result.referenceResistance, point.columnCapacitance);
    const double powerOn = BinarySenseAmpOperatingPowerFit(fit->node, point.resistanceOn,
        point.readVoltage, point.temperatureK);
    const double powerOff = BinarySenseAmpOperatingPowerFit(fit->node, point.resistanceOff,
        point.readVoltage, point.temperatureK);
    if (!Positive(result.inputCapacitance) || !Positive(coreArea) || !Positive(result.readLatency)
            || !Nonnegative(powerOn) || !Nonnegative(powerOff))
        return Reject(Status::Invalid, "nonphysical-sensing-fit");

    const double readVoltageSquared = point.readVoltage * point.readVoltage;
    const double vddSquared = technology.vdd * technology.vdd;
    const double columnEnergy = 2 * point.columnCapacitance * readVoltageSquared;
    // Binary (levelOutput-1=1), one unshared reference. Integer division and
    // multilevel reference loops from the original wrapper are absent.
    const double internalEnergy = (4 * result.gateCapacitanceN + 2 * result.junctionCapacitanceN)
        * readVoltageSquared
        + (6 * result.gateCapacitanceP + 5 * result.gateCapacitanceN
           + 4 * result.junctionCapacitanceP + 3 * result.junctionCapacitanceN) * vddSquared;
    const double operatingEnergy = std::max(powerOn, powerOff) * result.readLatency;

    // Fig.19(a): core 7N/4P, two two-inverter output buffers 4N/4P.
    // The helper returns (IoffN + IoffP)/2. Sum single-device off currents
    // without assuming stack suppression; Vread is disabled during standby.
    // This estimates subthreshold leakage, not gate tunnelling or a proven bound.
    const double offCurrentN = 2 * CalculateGateLeakage(INV, 1, widthN, 0, 300, technology);
    const double offCurrentP = 2 * CalculateGateLeakage(INV, 1, 0, widthP, 300, technology);
    const double standbyPower = technology.vdd * (11 * offCurrentN + 8 * offCurrentP);
    const double count = static_cast<double>(numAmplifiers);
    result.coreArea = coreArea * count;
    result.referenceArea = point.referenceColumnArea * count;
    result.area = result.coreArea + result.referenceArea;
    result.columnSwitchingEnergy = columnEnergy * count;
    result.internalSwitchingEnergy = internalEnergy * count;
    result.operatingEnergy = operatingEnergy * count;
    result.readDynamicEnergy = result.columnSwitchingEnergy + result.internalSwitchingEnergy + result.operatingEnergy;
    result.leakage = standbyPower * count;
    result.perAmplifierArea = coreArea + point.referenceColumnArea;
    result.perAmplifierReadDynamicEnergy = columnEnergy + internalEnergy + operatingEnergy;
    result.perAmplifierLeakage = standbyPower;
    if (!AllNonnegative({offCurrentN, offCurrentP, result.coreArea, result.referenceArea,
            result.area, result.columnSwitchingEnergy, result.internalSwitchingEnergy,
            result.operatingEnergy, result.readDynamicEnergy, result.leakage,
            result.perAmplifierArea, result.perAmplifierReadDynamicEnergy, result.perAmplifierLeakage}))
        return Reject(Status::Invalid, "nonphysical-ppa-result");
    result.status = Status::Applied;
    result.reason = "neurosim-binary-adaptation";
    return result;
}
