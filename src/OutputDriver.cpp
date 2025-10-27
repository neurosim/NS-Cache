//Copyright (c) 2015-2016, UT-Battelle, LLC. See LICENSE file in the top-level directory
// This file contains code from NVSim, (c) 2012-2013,  Pennsylvania State University 
//and Hewlett-Packard Company. See LICENSE_NVSim file in the top-level directory.
//No part of DESTINY Project, including this file, may be copied,
//modified, propagated, or distributed except according to the terms
//contained in the LICENSE file.


#include "OutputDriver.h"
#include "global.h"
#include "formula.h"
#include <math.h>

OutputDriver::OutputDriver() : FunctionUnit(){
	initialized = false;
	invalid = false;
}

OutputDriver::~OutputDriver() {
	// TODO Auto-generated destructor stub
}

void OutputDriver::Initialize(double _logicEffort, double _inputCap, double _outputCap, double _outputRes,
		bool _inv, BufferDesignTarget _areaOptimizationLevel, double _minDriverCurrent, bool _addRepeaters, double _wireLength) {
	if (initialized)
		cout << "[Output Driver] Warning: Already initialized!" << endl;

	logicEffort = _logicEffort;
	inputCap = _inputCap;
	outputCap = _outputCap;
	outputRes = _outputRes;
	inv = _inv;
	areaOptimizationLevel = _areaOptimizationLevel;
	minDriverCurrent = _minDriverCurrent;
	addRepeaters = _addRepeaters;
	wireLength - _wireLength;

	double sizingfactor_MUX = 1;
	
	double minNMOSDriverWidth = minDriverCurrent / tech->currentOnNmos[inputParameter->temperature - 300];
	minNMOSDriverWidth = MAX(MIN_NMOS_SIZE * tech->featureSize, minNMOSDriverWidth);

	if (minNMOSDriverWidth > inputParameter->maxNmosSize * tech->featureSize) {
		invalid = true;
		return;
	}

	int optimalNumStage;

	if (areaOptimizationLevel == latency_first) {
		double F = MAX(1, logicEffort * outputCap / inputCap);	/* Total logic effort */
		optimalNumStage = MAX(0, (int)(log(F) / log(OPT_F) + 0.5) - 1);

		if ((optimalNumStage % 2) ^ inv)	/* If odd, add 1 */
			optimalNumStage += 1;

		if (optimalNumStage > MAX_INV_CHAIN_LEN) {/* Exceed maximum stages */
			if (WARNING)
				cout << "[WARNING] Exceed maximum inverter chain length!" << endl;
			optimalNumStage = MAX_INV_CHAIN_LEN;
		}

			numStage = optimalNumStage;
			

		double f = pow(F, 1.0 / (optimalNumStage + 1));	/* Logic effort per stage */
		double inputCapLast = outputCap / f;

		widthNMOS[optimalNumStage-1] = MAX(MIN_NMOS_SIZE * tech->featureSize,
				inputCapLast / CalculateGateCap(1/*meter*/, *tech) / (1.0 + tech->pnSizeRatio));

		if (widthNMOS[optimalNumStage-1] > inputParameter->maxNmosSize * tech->featureSize /*|| _MUX*/) {
			if (WARNING)
				cout << "[WARNING] Exceed maximum NMOS size!" << endl;
			widthNMOS[optimalNumStage-1] = inputParameter->maxNmosSize * tech->featureSize;
			/* re-Calculate the logic effort */
			double capLastStage = CalculateGateCap((1 + tech->pnSizeRatio) * inputParameter->maxNmosSize * tech->featureSize, *tech);
			F = logicEffort * capLastStage / inputCap;
			f =	pow(F, 1.0 / (optimalNumStage));
		}

		if (widthNMOS[optimalNumStage-1] < minNMOSDriverWidth) {
			/* the last level Inv can not provide minimum current so that the Inv chain can't only decided by Logic Effort */
			areaOptimizationLevel = latency_area_trade_off;
		} else {
			widthPMOS[optimalNumStage-1] = widthNMOS[optimalNumStage-1] * tech->pnSizeRatio;

			for (int i = optimalNumStage-2; i >= 0; i--) {
				widthNMOS[i] = widthNMOS[i+1] / f;
				if (widthNMOS[i] < MIN_NMOS_SIZE * tech->featureSize) {
					if (WARNING)
						cout << "[WARNING] Exceed minimum NMOS size!" << endl;
					widthNMOS[i] = MIN_NMOS_SIZE * tech->featureSize;
				}
				widthPMOS[i] = widthNMOS[i] * tech->pnSizeRatio;
				EnlargeSize(&widthNMOS[i], &widthPMOS[i], tech->featureSize * MAX_TRANSISTOR_HEIGHT, *tech);
			}
		}
	}

	if (areaOptimizationLevel == latency_area_trade_off){
		double newOutputCap = CalculateGateCap(minNMOSDriverWidth, *tech) * (1.0 + tech->pnSizeRatio);
		double F = MAX(1, logicEffort * newOutputCap / inputCap);	/* Total logic effort */
		optimalNumStage = MAX(0, (int)(log(F) / log(OPT_F) + 0.5) - 1);

		if (!((optimalNumStage % 2) ^ inv))	/* If even, add 1 */
			optimalNumStage += 1;

		if (optimalNumStage > MAX_INV_CHAIN_LEN) {/* Exceed maximum stages */
			if (WARNING)
				cout << "[WARNING] Exceed maximum inverter chain length!" << endl;
			optimalNumStage = MAX_INV_CHAIN_LEN;
		}

		numStage = optimalNumStage + 1;

		widthNMOS[optimalNumStage] = minNMOSDriverWidth;
		widthPMOS[optimalNumStage] = widthNMOS[optimalNumStage] * tech->pnSizeRatio;

		double f = pow(F, 1.0 / (optimalNumStage + 1));	/* Logic effort per stage */

		for (int i = optimalNumStage - 1; i >= 0; i--) {
			widthNMOS[i] = widthNMOS[i+1] / f;
			if (widthNMOS[i] < MIN_NMOS_SIZE * tech->featureSize) {
				if (WARNING)
					cout << "[WARNING] Exceed minimum NMOS size!" << endl;
				widthNMOS[i] = MIN_NMOS_SIZE * tech->featureSize;
			}
			widthPMOS[i] = widthNMOS[i] * tech->pnSizeRatio;
		}
	} else if (areaOptimizationLevel == area_first) {
		optimalNumStage = 1;
		numStage = 1;
		widthNMOS[optimalNumStage - 1] = MAX(MIN_NMOS_SIZE * tech->featureSize, minNMOSDriverWidth);
		if (widthNMOS[optimalNumStage - 1] > AREA_OPT_CONSTRAIN * inputParameter->maxNmosSize * tech->featureSize) {
			invalid = true;
			return;
		}
		widthPMOS[optimalNumStage - 1] = widthNMOS[optimalNumStage - 1] * tech->pnSizeRatio;
	}

	/* Restore the original buffer design style */
	areaOptimizationLevel = _areaOptimizationLevel;

	/* Add Repeater Calculation Strategy */

	//unitLengthWireCap = localWire->capWirePerUnit;
	//unitLengthWireResistance = localWire->resWirePerUnit_M1;
//
	//// define min INV resistance and capacitance to calculate repeater size
	//widthMinInvN = MIN_NMOS_SIZE * tech->featureSize;
	//widthMinInvP = tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize;
	//CalculateGateArea(INV, 1, widthMinInvN, widthMinInvP, tech->featureSize * MAX_TRANSISTOR_HEIGHT, *tech, &hMinInv, &wMinInv);
	//CalculateGateCapacitance(INV, 1, widthMinInvN, widthMinInvP, hMinInv, *tech, &capMinInvInput, &capMinInvOutput);
	//// 1.4 update: change the formula
	//double resOnRep = (CalculateOnResistance(widthMinInvN, NMOS, inputParameter->temperature, *tech) + CalculateOnResistance(widthMinInvP, PMOS, inputParameter->temperature, *tech))/2;
	//
	//// optimal repeater design to achieve highest speed
	//repeaterSize = floor(sqrt(resOnRep*unitLengthWireCap/capMinInvInput/unitLengthWireResistance));
	//minDist = sqrt(2*resOnRep*(capMinInvOutput+capMinInvInput)/(unitLengthWireResistance*unitLengthWireCap));
	//CalculateGateArea(INV, 1, MIN_NMOS_SIZE * tech->featureSize * repeaterSize, tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize * repeaterSize, tech->featureSize * MAX_TRANSISTOR_HEIGHT, *tech, &hRep, &wRep);
	//CalculateGateCapacitance(INV, 1, MIN_NMOS_SIZE * tech->featureSize * repeaterSize, tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize * repeaterSize, hRep, *tech, &capRepInput, &capRepOutput);
	//// 1.4 update: change the formula
	//resOnRep = (CalculateOnResistance(MIN_NMOS_SIZE * tech->featureSize * repeaterSize, NMOS, inputParameter->temperature, *tech) + CalculateOnResistance(tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize * repeaterSize, PMOS, inputParameter->temperature, *tech))/2;
	//double minUnitLengthDelay = 0.7*(resOnRep*(capRepInput+capRepOutput+unitLengthWireCap*minDist)+0.54*unitLengthWireResistance*minDist*unitLengthWireCap*minDist+unitLengthWireResistance*minDist*capRepInput)/minDist;
	//double maxUnitLengthEnergy = (capRepInput+capRepOutput+unitLengthWireCap*minDist)*tech->vdd*tech->vdd/minDist;
	//
	//if (inputParameter->delaytolerance) {   // tradeoff: increase delay to decrease energy
	//	double delay = 0;
	//	double energy = 100;
	//	while ((delay<minUnitLengthDelay*(1+inputParameter->delaytolerance)) && (repeaterSize >= 1)) {
	//		repeaterSize -= 1;
	//		minDist *= 0.9;
	//		CalculateGateArea(INV, 1, MIN_NMOS_SIZE * tech->featureSize * repeaterSize, tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize * repeaterSize, tech->featureSize * MAX_TRANSISTOR_HEIGHT, *tech, &hRep, &wRep);
	//		CalculateGateCapacitance(INV, 1, MIN_NMOS_SIZE * tech->featureSize * repeaterSize, tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize * repeaterSize, hRep, *tech, &capRepInput, &capRepOutput);
	//		// 1.4 update: change the formula
	//		resOnRep = (CalculateOnResistance(MIN_NMOS_SIZE * tech->featureSize * repeaterSize, NMOS, inputParameter->temperature, *tech) + CalculateOnResistance(tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize * repeaterSize, PMOS, inputParameter->temperature, *tech))/2;
	//		delay = 0.7*(resOnRep*(capRepInput+capRepOutput+unitLengthWireCap*minDist)+0.54*unitLengthWireResistance*minDist*unitLengthWireCap*minDist+unitLengthWireResistance*minDist*capRepInput)/minDist;
	//		energy = (capRepInput+capRepOutput+unitLengthWireCap*minDist)*tech->vdd*tech->vdd/minDist;
	//	}
	//}
	//
	//widthInvN = MAX(1,repeaterSize) * MIN_NMOS_SIZE * tech->featureSize;
	//widthInvP = MAX(1,repeaterSize) * tech->pnSizeRatio * MIN_NMOS_SIZE * tech->featureSize;

	/*************************************/

	initialized = true;
}

void OutputDriver::CalculateArea() {
	if (!initialized) {
		cout << "[Output Driver] Error: Require initialization first!" << endl;
	} else if (invalid) {
		height = width = area = invalid_value;
	} else {
		double totalHeight = 0;
		double totalWidth = 0;
		double h, w;
		for (int i = 0; i < numStage; i++) {
			CalculateGateArea(INV, 1, widthNMOS[i], widthPMOS[i], tech->featureSize*40, *tech, &h, &w);
			totalHeight = MAX(totalHeight, h);
			totalWidth += w;
		}
		height = totalHeight;
		width = totalWidth;
		area = height * width;
	}
}

void OutputDriver::CalculateRC() {
	if (!initialized) {
		cout << "[Output Driver] Error: Require initialization first!" << endl;
	} else if (invalid) {
		;  // nothing to do if invalid
	} else if (numStage == 0) {
		capInput[0] = 0;
	} else {
		for (int i = 0; i < numStage; i++) {
			CalculateGateCapacitance(INV, 1, widthNMOS[i], widthPMOS[i], tech->featureSize * MAX_TRANSISTOR_HEIGHT, *tech, &(capInput[i]), &(capOutput[i]));
		}
	}
}

void OutputDriver::CalculateLatency(double _rampInput) {
	if (!initialized) {
		cout << "[Output Driver] Error: Require initialization first!" << endl;
	} else if (invalid) {
		readLatency = writeLatency = invalid_value;
	} else {
		rampInput = _rampInput;
		double resPullDown;
		double capLoad;
		double tr;	/* time constant */
		double gm;	/* transconductance */
		double beta;	/* for horowitz calculation */
		double temp;
		readLatency = 0;
		for (int i = 0; i < numStage - 1; i++) {
			resPullDown = CalculateOnResistance(((tech->featureSize <= 14*1e-9)? 2:1) * widthNMOS[i], NMOS, inputParameter->temperature, *tech);
			capLoad = capOutput[i] + capInput[i+1];
			tr = resPullDown * capLoad;
			gm = CalculateTransconductance(((tech->featureSize <= 14*1e-9)? 2:1) * widthNMOS[i], NMOS, *tech);
			beta = 1 / (resPullDown * gm);
			readLatency += horowitz(tr, beta, rampInput, &temp);
			rampInput = temp;	/* for next stage */
		}
		/* Last level inverter */
		resPullDown = CalculateOnResistance(((tech->featureSize <= 14*1e-9)? 2:1) * widthNMOS[numStage-1], NMOS, inputParameter->temperature, *tech);
		capLoad = capOutput[numStage-1] + outputCap;
		tr = resPullDown * capLoad + outputCap * outputRes / 2;
		gm = CalculateTransconductance(((tech->featureSize <= 14*1e-9)? 2:1) * widthNMOS[numStage-1], NMOS, *tech);
		beta = 1 / (resPullDown * gm);
		// if (MUX) readLatency += 0.69 * resPullDown * outputCap  + 0.38 * outputRes * outputCap;
		
		//double resOnRep = (CalculateOnResistance(widthInvN, NMOS, inputParameter->temperature, *tech) + CalculateOnResistance(widthInvP, PMOS, inputParameter->temperature, *tech))/2;
		
		//unitLatencyRep = 0.7*(resOnRep*(capInvInput+capInvOutput+unitLengthWireCap*minDist)+0.54*unitLengthWireResistance*minDist*unitLengthWireCap*minDist+unitLengthWireResistance*minDist*capInvInput)/minDist;
		//unitLatencyWire = 0.7*unitLengthWireResistance*minDist*unitLengthWireCap*minDist/minDist;

		//if(addRepeaters && numRepeater > 0){
		//	readLatency += wireLength*unitLatencyRep;
		//} else if (addRepeaters) {
		//	readLatency += wireLength*unitLatencyWire;
		//} else {
			readLatency += horowitz(tr, beta, rampInput, &rampOutput);
		//}
		
		
		rampInput = _rampInput;
		writeLatency = readLatency;
	}
}

void OutputDriver::CalculatePower() {
	if (!initialized) {
		cout << "[Output Driver] Error: Require initialization first!" << endl;
	} else if (invalid) {
		readDynamicEnergy = writeDynamicEnergy = leakage = invalid_value;
	} else {
		/* Leakage power */
		leakage = 0;
		for (int i = 0; i < numStage; i++) {
			leakage += CalculateGateLeakage(INV, 1, widthNMOS[i], widthPMOS[i], inputParameter->temperature, *tech)
					* tech->vdd;
		}

		//if(addRepeaters){
		//	unitLengthLeakage = CalculateGateLeakage(INV, 1, widthInvN, widthInvP, inputParameter->temperature, *tech) * tech->vdd / minDist;
		//	leakage += unitLengthLeakage * wireLength /* * numBitAccess */;
		//}

		/* Dynamic energy */
		readDynamicEnergy = 0;
		double capLoad;
		for (int i = 0; i < numStage - 1; i++) {
			capLoad = capOutput[i] + capInput[i+1];
			readDynamicEnergy += capLoad * tech->vdd * tech->vdd;
		}
		capLoad = capOutput[numStage-1] + outputCap;	/* outputCap here means the final load capacitance */
		readDynamicEnergy += capLoad * tech->vdd * tech->vdd;
		writeDynamicEnergy = readDynamicEnergy;

		//unitLengthEnergyRep = (capInvInput+capInvOutput+unitLengthWireCap*minDist)*tech->vdd*tech->vdd/minDist * 0.5 * /*param->outputtoggle*/ 0.5;
		//unitLengthEnergyWire = (unitLengthWireCap*minDist)*tech->vdd*tech->vdd/minDist * 0.5 * /*param->outputtoggle*/ 0.5;

		//if (numRepeater > 0 && addRepeaters) {
		//	readDynamicEnergy += wireLength*unitLengthEnergyRep;
		//} else if (addRepeaters) {
		//	readDynamicEnergy += wireLength*unitLengthEnergyWire;
		//}
	}
}

void OutputDriver::PrintProperty() {
	cout << "Output Driver Properties:" << endl;
	FunctionUnit::PrintProperty();
	cout << "Number of inverter stage: " << numStage << endl;
}

OutputDriver & OutputDriver::operator=(const OutputDriver &rhs) {
	//cout << "[PROGRESS] Line 322 :: OutputDriver.cc" << endl;
	height = rhs.height;
	width = rhs.width;
	area = rhs.area;
	readLatency = rhs.readLatency;
	writeLatency = rhs.writeLatency;
	readDynamicEnergy = rhs.readDynamicEnergy;
	writeDynamicEnergy = rhs.writeDynamicEnergy;
	resetLatency = rhs.resetLatency;
	setLatency = rhs.setLatency;
	resetDynamicEnergy = rhs.resetDynamicEnergy;
	setDynamicEnergy = rhs.setDynamicEnergy;
	cellReadEnergy = rhs.cellReadEnergy;
	cellSetEnergy = rhs.cellSetEnergy;
	cellResetEnergy = rhs.cellResetEnergy;
	leakage = rhs.leakage;
	initialized = rhs.initialized;
	invalid = rhs.invalid;
	logicEffort = rhs.logicEffort;
	inputCap = rhs.inputCap;
	outputCap = rhs.outputCap;
	outputRes = rhs.outputRes;
	inv = rhs.inv;
	numStage = rhs.numStage;
	areaOptimizationLevel = rhs.areaOptimizationLevel;
	minDriverCurrent = rhs.minDriverCurrent;
	rampInput = rhs.rampInput;
	rampOutput = rhs.rampOutput;

	return *this;
}
