//Copyright (c) 2015-2016, UT-Battelle, LLC. See LICENSE file in the top-level directory
// This file contains code from NVSim, (c) 2012-2013,  Pennsylvania State University 
//and Hewlett-Packard Company. See LICENSE_NVSim file in the top-level directory.
//No part of DESTINY Project, including this file, may be copied,
//modified, propagated, or distributed except according to the terms
//contained in the LICENSE file.


#include "RowDecoder.h"
#include "formula.h"
#include "global.h"

RowDecoder::RowDecoder()
	: FunctionUnit(),
	  initialized(false),
	  invalid(false),
	  outputDriver(),
	  numRow(0),
	  multipleRowPerSet(false),
	  numNandInput(0),
	  capLoad(0),
	  resLoad(0),
	  areaOptimizationLevel(latency_first),
	  minDriverCurrent(0),
	  widthNandN(0),
	  widthNandP(0),
	  capNandInput(0),
	  capNandOutput(0),
	  rampInput(0),
	  rampOutput(0),
	  wireLength(0) {
}

RowDecoder::~RowDecoder() {
	// TODO Auto-generated destructor stub
}

void RowDecoder::Initialize(int _numRow, double _capLoad, double _resLoad,
		bool _multipleRowPerSet, BufferDesignTarget _areaOptimizationLevel, double _minDriverCurrent, bool _MUX, double _wireLength) {
	if (initialized)
		cout << "[Row Decoder] Warning: Already initialized!" << endl;

	numRow = _numRow;
	capLoad = _capLoad;
	resLoad = _resLoad;
	multipleRowPerSet = _multipleRowPerSet;
	areaOptimizationLevel = _areaOptimizationLevel;
	minDriverCurrent = _minDriverCurrent;
	wireLength = _wireLength;

	if (numRow <= 8) {	/* The predecoder output is used directly */
		if (multipleRowPerSet)
			numNandInput = 2;	/* NAND way-select with predecoder output */
		else
			numNandInput = 0;	/* no circuit needed */
	} else {
		if (multipleRowPerSet)
			numNandInput = 3;	/* NAND way-select with two predecoder outputs */
		else
			numNandInput = 2;	/* just NAND two predecoder outputs */
	}

	if (numNandInput > 0) {
		double logicEffortNand;
		double capNand;
		if (numNandInput == 2) {	/* NAND2 */
			widthNandN = 2 * MIN_NMOS_SIZE * tech->featureSize;
			logicEffortNand = (2+tech->pnSizeRatio) / (1+tech->pnSizeRatio);
		} else {					/* NAND3 */
			widthNandN = 3 * MIN_NMOS_SIZE * tech->featureSize;
			logicEffortNand = (3+tech->pnSizeRatio) / (1+tech->pnSizeRatio);
		}
		widthNandP = tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize;
		EnlargeSize(&widthNandN, &widthNandP, tech->featureSize * MAX_TRANSISTOR_HEIGHT, *tech);
		capNand = CalculateGateCap(((tech->featureSize <= 14*1e-9)? 2:1) * widthNandN, *tech) + CalculateGateCap(((tech->featureSize <= 14*1e-9)? 2:1) * widthNandP, *tech);
		outputDriver.Initialize(logicEffortNand, capNand, capLoad, resLoad, true, latency_first, minDriverCurrent, _MUX /*false*/, wireLength);
	} else {
		/* we only need an 1-level output buffer to driver the wordline */
		double capInv;
		widthNandN = MIN_NMOS_SIZE * tech->featureSize;
		widthNandP = tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize;
		EnlargeSize(&widthNandN, &widthNandP, tech->featureSize * MAX_TRANSISTOR_HEIGHT, *tech);
		capInv = CalculateGateCap(((tech->featureSize <= 14*1e-9)? 2:1) * widthNandN, *tech) + CalculateGateCap(((tech->featureSize <= 14*1e-9)? 2:1) * widthNandP, *tech);
		outputDriver.Initialize(1, capInv, capLoad, resLoad, true, latency_first, minDriverCurrent, _MUX /*false*/, wireLength);
	}

	if (outputDriver.invalid) {
		invalid = true;
		return;
	}

	initialized = true;
}

void RowDecoder::CalculateArea() {
	if (!initialized) {
		cout << "[Row Decoder Area] Error: Require initialization first!" << endl;
	} else {
		outputDriver.CalculateArea();
		if (numNandInput == 0) {	/* no circuit needed, use predecoder outputs directly */
			height = outputDriver.height;
			width = outputDriver.width;
		} else {
			double hNand, wNand;
			CalculateGateArea(NAND, numNandInput, widthNandN, widthNandP, tech->featureSize*40, *tech, &hNand, &wNand);
			height = MAX(hNand, outputDriver.height);
			width = wNand + outputDriver.width;
		}
		height *= numRow;
		area = height * width;
	}
}

void RowDecoder::CalculateRC() {
	if (!initialized) {
		cout << "[Row Decoder RC] Error: Require initialization first!" << endl;
	} else {
		outputDriver.CalculateRC();
		if (numNandInput == 0) {	/* no circuit needed, use predecoder outputs directly */
			capNandInput = capNandOutput = 0;
		} else {
			CalculateGateCapacitance(NAND, numNandInput, widthNandN, widthNandP, tech->featureSize * MAX_TRANSISTOR_HEIGHT, *tech, &capNandInput, &capNandOutput);
		}
	}
}

void RowDecoder::CalculateLatency(double _rampInput) {
	if (!initialized) {
		cout << "[Row Decoder Latency] Error: Require initialization first!" << endl;
	} else {
		if (numNandInput == 0) {	/* no circuit needed, use predecoder outputs directly */
			outputDriver.CalculateLatency(_rampInput);
			readLatency = writeLatency = outputDriver.readLatency;
			rampOutput = outputDriver.rampOutput;
		} else {
			rampInput = _rampInput;

			double resPullDown;
			double capLoad;
			double tr;	/* time constant */
			double gm;	/* transconductance */
			double beta;	/* for horowitz calculation */
			double rampInputForDriver;

			resPullDown = CalculateOnResistance(((tech->featureSize <= 14*1e-9)? 2:1) * widthNandN, NMOS, inputParameter->temperature, *tech) * numNandInput;
			capLoad = capNandOutput + outputDriver.capInput[0];
			tr = resPullDown * capLoad;
			gm = CalculateTransconductance(((tech->featureSize <= 14*1e-9)? 2:1) * widthNandN, NMOS, *tech);
			beta = 1 / (resPullDown * gm);
			readLatency = horowitz(tr, beta, rampInput, &rampInputForDriver);

		//	cout << "RowDecoder Internal Latency: " << readLatency * 1e9 << endl;

			outputDriver.CalculateLatency(rampInputForDriver);
			readLatency += outputDriver.readLatency;

		//	cout << "RowDecoder Driver Latency %: " << outputDriver.readLatency * 1e9 << endl;

			writeLatency = readLatency;
			rampOutput = outputDriver.rampOutput;
		}
	}
}

void RowDecoder::CalculatePower() {
	if (!initialized) {
		cout << "[Row Decoder Power] Error: Require initialization first!" << endl;
	} else {
		outputDriver.CalculatePower();
		leakage = outputDriver.leakage;
		if (numNandInput == 0) {	/* no circuit needed, use predecoder outputs directly */
			readDynamicEnergy = writeDynamicEnergy = outputDriver.readDynamicEnergy;
		} else {
			/* Leakage power */
			leakage += CalculateGateLeakage(NAND, numNandInput, widthNandN, widthNandP,
					inputParameter->temperature, *tech) * tech->vdd;
			/* Dynamic energy */
			double capLoad = capNandOutput + outputDriver.capInput[0];
            /* For DRAM types account for overdriven wordline. */
            if (cell->memCellType == DRAM || cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
                readDynamicEnergy = capLoad * tech->vpp * tech->vpp;
            } else {
                readDynamicEnergy = capLoad * tech->vdd * tech->vdd;
            }
			readDynamicEnergy += outputDriver.readDynamicEnergy;
			readDynamicEnergy *= 1;	/* only one row is activated each time */
			writeDynamicEnergy = readDynamicEnergy;
		}
		leakage *= numRow;
	}
}

void RowDecoder::PrintProperty() {
	cout << "Row Decoder Properties:" << endl;
	FunctionUnit::PrintProperty();
}

RowDecoder & RowDecoder::operator=(const RowDecoder &rhs) = default;
