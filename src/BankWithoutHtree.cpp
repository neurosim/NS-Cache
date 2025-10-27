/*******************************************************************************
* Copyright (c) 2025
* Georgia Institute of Technology
* 
* This source code is part of NeuroSim (NS)-Cache - a framework developed for early
* exploration of cache memories in advanced technology nodes (FinFET, nanosheet, CFET generations).
* The tool extends previously developed Destiny, NVSim, and Cacti3dd. (Copyright Information Below)
* Copyright of the model is maintained by the developers, and the model is distributed under 
* the terms of the Creative Commons Attribution-NonCommercial 4.0 International Public License (see LICENSE)
*******************************************************************************/

//Copyright (c) 2015-2016, UT-Battelle, LLC. See LICENSE file in the top-level directory
// This file contains code from NVSim, (c) 2012-2013,  Pennsylvania State University 
//and Hewlett-Packard Company. See LICENSE_NVSim file in the top-level directory.
//No part of DESTINY Project, including this file, may be copied,
//modified, propagated, or distributed except according to the terms
//contained in the LICENSE file.


#include "BankWithoutHtree.h"
#include "formula.h"
#include "global.h"
#include <math.h>

BankWithoutHtree::BankWithoutHtree() {
	// TODO Auto-generated constructor stub
	initialized = false;
	invalid = false;
}

BankWithoutHtree::~BankWithoutHtree() {
	// TODO Auto-generated destructor stub
}


void BankWithoutHtree::Initialize(int _numRowSubArray, int _numColumnSubArray, long long _capacity,
		long _blockSize, int _associativity, int _numRowPerSet, int _numActiveSubArrayPerRow,
		int _numActiveSubArrayPerColumn, int _muxSenseAmp, bool _internalSenseAmp, int _muxOutputLev1, int _muxOutputLev2,
		int _numRowMat, int _numColumnMat,
		int _numActiveMatPerRow, int _numActiveMatPerColumn,
		BufferDesignTarget _areaOptimizationLevel, MemoryType _memoryType,
        int _stackedDieCount, int _partitionGranularity, int monolithicStackCount) {
	if (initialized) {
		/* Reset the class for re-initialization */
		initialized = false;
		invalid = false;
	}

	if (!_internalSenseAmp) {
		if (cell->memCellType == DRAM || cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
			invalid = true;
			cout << "[BankWithoutHtree] Error: DRAM does not support external sense amplification!" << endl;
			return;
		} else if (globalWire->wireRepeaterType != repeated_none) {
			invalid = true;
			initialized = true;
			return;
		}
	}

	numRowSubArray = _numRowSubArray;
	numColumnSubArray = _numColumnSubArray;
	capacity = _capacity;
	blockSize = _blockSize;
	associativity = _associativity;
	numRowPerSet = _numRowPerSet;
	internalSenseAmp = _internalSenseAmp;
	areaOptimizationLevel = _areaOptimizationLevel;
	memoryType = _memoryType;
    stackedDieCount = _stackedDieCount;
    partitionGranularity = _partitionGranularity;
	numWay = 1;	/* default value for non-cache design */

	/* Calculate the physical signals that are required in routing. Use double during the calculation to avoid overflow */
    if (stackedDieCount > 1 /*&& partitionGranularity == 0*/) {
        numAddressBit = (int)(log2((double)capacity / blockSize / associativity / stackedDieCount) + 0.1);
    } else {
        numAddressBit = (int)(log2((double)capacity / blockSize / associativity) + 0.1);
    }

	if (_numActiveSubArrayPerRow > numColumnSubArray) {
		cout << "[Bank] Warning: The number of active mat per row is larger than the number of mat per row!"  << endl;
		cout << _numActiveSubArrayPerRow << " > " << numColumnSubArray << endl;
		numActiveSubArrayPerRow = numColumnSubArray;
	} else {
		numActiveSubArrayPerRow = _numActiveSubArrayPerRow;
	}
	if (_numActiveSubArrayPerColumn > numRowSubArray) {
		cout << "[Bank] Warning: The number of active mat per column is larger than the number of mat per column!"  << endl;
		cout << _numActiveSubArrayPerColumn << " > " << numRowSubArray << endl;
		numActiveSubArrayPerColumn = numRowSubArray;
	} else {
		numActiveSubArrayPerColumn = _numActiveSubArrayPerColumn;
	}
	muxSenseAmp = _muxSenseAmp;
	muxOutputLev1 = _muxOutputLev1;
	muxOutputLev2 = _muxOutputLev2;

	numRowMat = _numRowMat;
	numColumnMat = _numColumnMat;
	if (_numActiveMatPerRow > numColumnMat) {
		cout << "[Bank] Warning: The number of active mat per row is larger than the number of mat per row!"  << endl;
		cout << _numActiveMatPerRow << " > " << numColumnMat << endl;
		numActiveMatPerRow = numColumnMat;
	} else {
		numActiveMatPerRow = _numActiveMatPerRow;
	}
	if (_numActiveMatPerColumn > numRowMat) {
		cout << "[Bank] Warning: The number of active mat per column is larger than the number of mat per column!"  << endl;
		cout << _numActiveMatPerColumn << " > " << numRowMat << endl;
		numActiveMatPerColumn = numRowMat;
	} else {
		numActiveMatPerColumn = _numActiveMatPerColumn;
	}

	/* The number of address bits that are used to power gate inactive subarrays */
	int numAddressForGating = (int)(log2(numRowSubArray * numColumnSubArray / numActiveSubArrayPerColumn / numActiveSubArrayPerRow)+0.1);
	numAddressBitRouteToSubArray = numAddressBit - numAddressForGating;	/* Only use the effective address bits in the following calculation */
	numDataBitRouteToSubArray = blockSize;


	if (memoryType == data) { /* Data array */
		numDataBitRouteToSubArray = blockSize / numActiveSubArrayPerColumn / numActiveSubArrayPerRow;
		if (numRowPerSet > associativity) {
			/* There is no enough ways to distribute into multiple rows */
			invalid = true;
			initialized = true;
			return;
		}
		numWay = associativity;
		int numWayPerRow = numWay / numRowPerSet;	/* At least 1, otherwise it is invalid, and returned already */
		if (numWayPerRow > 1) {		/* multiple ways per row, needs extra mux level */
			/* Do mux level recalculation to contain the multiple ways */
			if (cell->memCellType == DRAM || cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
				/* for DRAM, mux before sense amp has to be 1, only mux output1 and mux output2 can be used */
				int numWayPerRowInLog = (int)(log2((double)numWayPerRow) + 0.1);
				int extraMuxOutputLev2 = (int)pow(2, numWayPerRowInLog / 2);
				int extraMuxOutputLev1 = numWayPerRow / extraMuxOutputLev2;
				muxOutputLev1 *= extraMuxOutputLev1;
				muxOutputLev2 *= extraMuxOutputLev2;
			} else {
				/* for non-DRAM, all mux levels can be used */
				int numWayPerRowInLog = (int)(log2((double)numWayPerRow) + 0.1);
				int extraMuxOutputLev2 = (int)pow(2, numWayPerRowInLog / 3);
				int extraMuxOutputLev1 = extraMuxOutputLev2;
				int extraMuxSenseAmp = numWayPerRow / extraMuxOutputLev1 / extraMuxOutputLev2;
				muxSenseAmp *= extraMuxSenseAmp;
				muxOutputLev1 *= extraMuxOutputLev1;
				muxOutputLev2 *= extraMuxOutputLev2;
			}
		}
	} else if (memoryType == tag) { /* Tag array */
		if (numRowPerSet > 1) {
			/* tag array cannot have multiple rows to contain ways in a set, otherwise the bitline has to be shared */
			invalid = true;
			initialized = true;
			return;
		}
		numDataBitRouteToSubArray = blockSize;
		numWay = associativity / numActiveSubArrayPerColumn / numActiveSubArrayPerRow;
		if (numWay < 1) {
			/* This subarray does not contain at least one way */
			invalid = true;
			initialized = true;
			return;
		}
	} else { /* CAM */
		numDataBitRouteToSubArray = blockSize;
		numWay = 1;
	}


    // NVSIM3D - Give each subarray only the number of mats per layer
	//subarray.Initialize(numRowMat, numColumnMat, numAddressBitRouteToSubArray, numDataBitRouteToSubArray,
	//		numWay, numRowPerSet, false, numActiveMatPerRow, numActiveMatPerColumn,
	//		muxSenseAmp, internalSenseAmp, muxOutputLev1, muxOutputLev2, areaOptimizationLevel, memoryType);
	subarray.Initialize(numRowMat, numColumnMat, numAddressBitRouteToSubArray, numDataBitRouteToSubArray,
			numWay, numRowPerSet, false, numActiveMatPerRow, numActiveMatPerColumn,
			muxSenseAmp, internalSenseAmp, muxOutputLev1, muxOutputLev2, areaOptimizationLevel, memoryType, 
            stackedDieCount, partitionGranularity, monolithicStackCount);
	/* Check if subarray is under a legal configuration */
	if (subarray.invalid) {
		invalid = true;
		initialized = true;
		return;
	}

	subarray.CalculateArea();

	if (!internalSenseAmp) {
		bool voltageSense = true;
		double senseVoltage;
		senseVoltage = cell->minSenseVoltage;
		if (cell->memCellType == SRAM) {
			/* SRAM, DRAM, and eDRAM all use voltage sensing */
			voltageSense = true;
		} else if (cell->memCellType == MRAM || cell->memCellType == PCRAM || cell->memCellType == memristor || cell->memCellType == FBRAM) {
			voltageSense = cell->readMode;
		} else {/* NAND flash */
			// TO-DO
		}

		int numSenseAmp;
		if (memoryType == data)
			numSenseAmp = blockSize;
		else
			numSenseAmp = blockSize * associativity;

		globalSenseAmp.Initialize(numSenseAmp, !voltageSense, senseVoltage, subarray.width * numColumnSubArray / numSenseAmp);
		if (globalSenseAmp.invalid) {
			invalid = true;
			initialized = true;
			return;
		}
		globalSenseAmp.CalculateRC();
		globalBitlineMux.Initialize(numRowSubArray * numColumnSubArray / numActiveSubArrayPerColumn / numActiveSubArrayPerRow, numSenseAmp, globalSenseAmp.capLoad, globalSenseAmp.capLoad, 0);
		globalBitlineMux.CalculateRC();

		if (memoryType == tag)
			globalComparator.Initialize(blockSize, 0 /* TO-DO: only for test */);
	}

	/* Reset the mux values for correct printing */
	muxSenseAmp = _muxSenseAmp;
	muxOutputLev1 = _muxOutputLev1;
	muxOutputLev2 = _muxOutputLev2;

    /* Initialize TSV connections. */
    if (stackedDieCount > 1 /*&& partitionGranularity == 0*/) {
        TSV_type tsv_type = tech->WireTypeToTSVType(inputParameter->maxGlobalWireType);
        tsvArray.Initialize(tsv_type);
    }

	initialized = true;
}

void BankWithoutHtree::CalculateArea() {
	if (!initialized) {
		cout << "[BankWithoutHtree] Error: Require initialization first!" << endl;
	} else if (invalid) {
		height = width = area = invalid_value;
	} else {
		height = subarray.height * numRowSubArray;
		width = subarray.width * numColumnSubArray;

		int numWireSharingWidth;
		double effectivePitch;
		if (globalWire->wireRepeaterType == repeated_none) {
			numWireSharingWidth = 1;
			effectivePitch = 0;		/* assume that the wire is built on another metal layer, there does not cause silicon area */
			//effectivePitch = globalWire->wirePitch;
		} else {
			numWireSharingWidth = (int)floor(globalWire->repeaterSpacing / globalWire->repeaterHeight);
			effectivePitch = globalWire->repeatedWirePitch;
		}

		width += ceil((double)numRowSubArray * numColumnSubArray * numAddressBitRouteToSubArray / numWireSharingWidth) * effectivePitch;

		if (!internalSenseAmp) {
			globalSenseAmp.CalculateArea();
			height += globalSenseAmp.height;
			globalBitlineMux.CalculateArea();
			height += globalBitlineMux.height;
			if (memoryType == tag) {
				globalComparator.CalculateArea();
				height += associativity * globalComparator.area / width;
			}
		}

		/* Determine if the aspect ratio meets the constraint */
		if (memoryType == data)
			if (height / width > CONSTRAINT_ASPECT_RATIO_BANK || width / height > CONSTRAINT_ASPECT_RATIO_BANK) {
				/* illegal */
				invalid = true;
				height = width = area = invalid_value;
				return;
			}

		area = height * width;

        /* Initialize TSV connections. */
        if (stackedDieCount > 1 /*&& partitionGranularity == 0*/) {
            tsvArray.CalculateArea();

            //int numControlBits = (int)(log2((double)stackedDieCount + 0.1));
            int numControlBits = stackedDieCount;
            int numAddressBits = (int)(log2((double)capacity / blockSize / associativity / stackedDieCount) + 0.1);
            int numDataBits = blockSize * 2; // Read and write TSVs

            // Fine-granularity has predecoders on logic layer
            if (partitionGranularity == 1) {
                numAddressBits = 0;
            }

            double redundancyFactor = inputParameter->tsvRedundancy;
            tsvArray.numTotalBits = (int)((double)(numControlBits + numAddressBits + numDataBits) * redundancyFactor);
            tsvArray.numAccessBits = (int)((double)(numControlBits + numAddressBits + blockSize) * redundancyFactor);

            // We're not adding in a particular dimension (width/height) so increase the total
            area += tsvArray.numTotalBits * tsvArray.area;
        }
	}
}

void BankWithoutHtree::CalculateRC() {
	if (!initialized) {
		cout << "[BankWithoutHtree] Error: Require initialization first!" << endl;
	} else if (!invalid) {
		subarray.CalculateRC();
		if (!internalSenseAmp) {
			globalBitlineMux.CalculateRC();
			globalSenseAmp.CalculateRC();
			if (memoryType == tag)
				globalComparator.CalculateRC();
		}
	}
}

void BankWithoutHtree::CalculateLatencyAndPower() {
	if (!initialized) {
		cout << "[BankWithoutHtree] Error: Require initialization first!" << endl;
	} else if (invalid) {
		readLatency = writeLatency = invalid_value;
		readDynamicEnergy = writeDynamicEnergy = invalid_value;
		leakage = invalid_value;
	} else {
		double latency = 0;
		double energy = 0;
		double leakageWire = 0;

		subarray.CalculateLatency(infinite_ramp);
		subarray.CalculatePower();
		readLatency = resetLatency = setLatency = writeLatency = 0;
        refreshLatency = subarray.refreshLatency * numColumnSubArray; // TOTAL refresh time for all SubArrays
		readDynamicEnergy = writeDynamicEnergy = resetDynamicEnergy = setDynamicEnergy = 0;
        refreshDynamicEnergy = subarray.refreshDynamicEnergy * numRowSubArray * numColumnSubArray;
		leakage = 0;

		double lengthWire;
		lengthWire = subarray.height * (numRowSubArray + 1);
		for (int i = 0; i < numRowSubArray; i++) {
			lengthWire -= subarray.height;
			if (internalSenseAmp) {
				double numBitRouteToSubArray = 0;
				globalWire->CalculateLatencyAndPower(lengthWire, &latency, &energy, &leakageWire);
				if (i == 0){
					readLatency += latency;
					writeLatency += latency;
                    refreshLatency += latency;
				}
				if (i < numActiveSubArrayPerColumn) {
					if (memoryType == tag)
						numBitRouteToSubArray = numAddressBitRouteToSubArray + numDataBitRouteToSubArray + numWay;
					else
						numBitRouteToSubArray = numAddressBitRouteToSubArray + numDataBitRouteToSubArray;
					readDynamicEnergy += energy * numBitRouteToSubArray * numActiveSubArrayPerRow;
					writeDynamicEnergy += energy * numBitRouteToSubArray * numActiveSubArrayPerRow;
                    refreshDynamicEnergy += energy * numBitRouteToSubArray * numActiveSubArrayPerRow;
				}
				leakage += leakageWire * numBitRouteToSubArray * numColumnSubArray;
			} else {
				double resLocalBitline, capLocalBitline, resBitlineMux, capBitlineMux;
				capBitlineMux = globalBitlineMux.capNMOSPassTransistor;
				resBitlineMux = globalBitlineMux.resNMOSPassTransistor;
				resLocalBitline = subarray.mat.resBitline + 3 * resBitlineMux;
				capLocalBitline = subarray.mat.capBitline + 6 * capBitlineMux;
				double resGlobalBitline, capGlobalBitline;
				resGlobalBitline = lengthWire * globalWire->resWirePerUnit;
				capGlobalBitline = lengthWire * globalWire->capWirePerUnit;
				double capGlobalBitlineMux;
				capGlobalBitlineMux = globalBitlineMux.capForPreviousDelayCalculation;
				if (cell->memCellType == SRAM) {
					double vpre = cell->readVoltage;	/* This value should be equal to resetVoltage and setVoltage for SRAM */
					if (i == 0) {
						latency = resLocalBitline * capGlobalBitline / 2 +
								(resLocalBitline + resGlobalBitline) * (capGlobalBitline / 2 + capGlobalBitlineMux);
						latency *= log(vpre / (vpre - globalSenseAmp.senseVoltage));
						latency += resLocalBitline * capGlobalBitline / 2;
						globalBitlineMux.CalculateLatency(1e20);
						latency += globalBitlineMux.readLatency;
						globalSenseAmp.CalculateLatency(1e20);
						writeLatency += latency;
						latency += globalSenseAmp.readLatency;
						readLatency += latency;
					}
					if (i <  numActiveSubArrayPerColumn) {
						energy = capGlobalBitline * tech->vdd * tech->vdd * numAddressBitRouteToSubArray;
						readDynamicEnergy += energy;
						writeDynamicEnergy += energy;
						readDynamicEnergy += capGlobalBitline * vpre * vpre * numWay;
						writeDynamicEnergy += capGlobalBitline * vpre * vpre * numDataBitRouteToSubArray;
					}
				} else if (cell->memCellType == MRAM || cell->memCellType == PCRAM || cell->memCellType == memristor || cell->memCellType == FBRAM) {
					double vWrite = MAX(fabs(cell->resetVoltage), fabs(cell->setVoltage));
					double tau, latencyOff, latencyOn;
					double vPre = subarray.mat.voltagePrecharge;
					double vOn = subarray.mat.voltageMemCellOn;
					double vOff = subarray.mat.voltageMemCellOff;
					if (i == 0) {
						tau = resBitlineMux * capGlobalBitline / 2 + (resBitlineMux + resGlobalBitline)
								* (capGlobalBitline + capLocalBitline) / 2 + (resBitlineMux + resGlobalBitline
										+ resLocalBitline) * capLocalBitline / 2;
						writeLatency += 0.63 * tau;
						if (cell->readMode == false) {	/* current-sensing */
							/* Use ICCAD 2009 model */
							resLocalBitline += subarray.mat.resMemCellOff;
							tau = resGlobalBitline * capGlobalBitline / 2 *
									(resLocalBitline + resGlobalBitline / 3) / (resLocalBitline + resGlobalBitline);
							readLatency += 0.63 * tau;
						} else {						/* voltage-sensing */
							if (cell->readVoltage == 0) {  /* Current-in voltage sensing */
								resLocalBitline += subarray.mat.resMemCellOn;
								tau = resLocalBitline * capGlobalBitline + (resLocalBitline + resGlobalBitline) * capGlobalBitline / 2;
								latencyOn = tau * log((vPre - vOn)/(vPre - vOn - globalSenseAmp.senseVoltage));
								resLocalBitline += cell->resistanceOff - cell->resistanceOn;
								tau = resLocalBitline * capGlobalBitline + (resLocalBitline + resGlobalBitline) * capGlobalBitline / 2;
								latencyOff = tau * log((vOff - vPre)/(vOff - vPre - globalSenseAmp.senseVoltage));
							} else {   /*Voltage-in voltage sensing */
								resLocalBitline += subarray.mat.resEquivalentOn;
								tau = resLocalBitline * capGlobalBitline + (resLocalBitline + resGlobalBitline) * capGlobalBitline / 2;
								latencyOn = tau * log((vPre - vOn)/(vPre - vOn - globalSenseAmp.senseVoltage));
								resLocalBitline += subarray.mat.resEquivalentOff - subarray.mat.resEquivalentOn;
								tau = resLocalBitline * capGlobalBitline + (resLocalBitline + resGlobalBitline) * capGlobalBitline / 2;
								latencyOff = tau * log((vOff - vPre)/(vOff - vPre - globalSenseAmp.senseVoltage));
							}
							readLatency -= subarray.mat.bitlineDelay;
							if ((latencyOn + subarray.mat.bitlineDelayOn) > (latencyOff + subarray.mat.bitlineDelayOff))
								readLatency += latencyOn + subarray.mat.bitlineDelayOn;
							else
								readLatency += latencyOff + subarray.mat.bitlineDelayOff;
						}
					}
					if (i <  numActiveSubArrayPerColumn) {
						energy = capGlobalBitline * tech->vdd * tech->vdd * numAddressBitRouteToSubArray;
						readDynamicEnergy += energy;
						writeDynamicEnergy += energy;
						writeDynamicEnergy += capGlobalBitline * vWrite * vWrite * numDataBitRouteToSubArray;
						if (cell->readMode) { /*Voltage-in voltage sensing */
							readDynamicEnergy += capGlobalBitline * (vPre * vPre - vOn * vOn )* numDataBitRouteToSubArray;
						}
					}
				}

			}
		}
		if (!internalSenseAmp) {
			globalBitlineMux.CalculateLatency(1e40);
			globalSenseAmp.CalculateLatency(1e40);
			readLatency += globalBitlineMux.readLatency + globalSenseAmp.readLatency;
			writeLatency += globalBitlineMux.writeLatency + globalSenseAmp.writeLatency;
			globalBitlineMux.CalculatePower();
			globalSenseAmp.CalculatePower();
			readDynamicEnergy += (globalBitlineMux.readDynamicEnergy + globalSenseAmp.readDynamicEnergy) * numActiveSubArrayPerRow;
			writeDynamicEnergy += (globalBitlineMux.writeDynamicEnergy + globalSenseAmp.writeDynamicEnergy) * numActiveSubArrayPerRow;
			leakage += (globalBitlineMux.leakage + globalSenseAmp.leakage) * numColumnSubArray;
			if (memoryType == tag) {
				globalComparator.CalculateLatency(1e40);
				readLatency += globalComparator.readLatency;
				globalComparator.CalculatePower();
				readDynamicEnergy += numWay * globalComparator.readDynamicEnergy;
				leakage += associativity * globalComparator.leakage;
			}
		}
	}

	/* only 1/A wires are activated in fast mode cache write */
	if (inputParameter->designTarget == cache && inputParameter->cacheAccessMode == fast_access_mode)
		writeDynamicEnergy /= inputParameter->associativity;

	readLatency += subarray.readLatency;
	resetLatency = writeLatency + subarray.resetLatency;
	setLatency = writeLatency + subarray.setLatency;
	writeLatency += subarray.writeLatency;
	readDynamicEnergy += subarray.readDynamicEnergy * numActiveSubArrayPerRow * numActiveSubArrayPerColumn;
	cellReadEnergy = subarray.cellReadEnergy * numActiveSubArrayPerRow * numActiveSubArrayPerColumn;
	cellSetEnergy = subarray.cellSetEnergy * numActiveSubArrayPerRow * numActiveSubArrayPerColumn;
	cellResetEnergy = subarray.cellResetEnergy * numActiveSubArrayPerRow * numActiveSubArrayPerColumn;
	resetDynamicEnergy = writeDynamicEnergy + subarray.resetDynamicEnergy * numActiveSubArrayPerRow * numActiveSubArrayPerColumn;
	setDynamicEnergy = writeDynamicEnergy + subarray.setDynamicEnergy * numActiveSubArrayPerRow * numActiveSubArrayPerColumn;
	writeDynamicEnergy += subarray.writeDynamicEnergy * numActiveSubArrayPerRow * numActiveSubArrayPerColumn;
	leakage += subarray.leakage * numRowSubArray * numColumnSubArray;

    /* Why is this stuff outside the else conditional? */
    routingReadLatency = readLatency - subarray.readLatency;
    routingWriteLatency = writeLatency - subarray.writeLatency;
    routingResetLatency = resetLatency - subarray.resetLatency;
    routingSetLatency = setLatency - subarray.setLatency;
    routingRefreshLatency = refreshLatency - subarray.refreshLatency;

    routingReadDynamicEnergy = readDynamicEnergy - subarray.readDynamicEnergy * numActiveSubArrayPerColumn * numActiveSubArrayPerRow;
    routingWriteDynamicEnergy = writeDynamicEnergy - subarray.writeDynamicEnergy * numActiveSubArrayPerColumn * numActiveSubArrayPerRow;
    routingResetDynamicEnergy = resetDynamicEnergy - subarray.resetDynamicEnergy * numActiveSubArrayPerColumn * numActiveSubArrayPerRow;
    routingSetDynamicEnergy = setDynamicEnergy - subarray.setDynamicEnergy * numActiveSubArrayPerColumn * numActiveSubArrayPerRow;
    routingRefreshDynamicEnergy = refreshDynamicEnergy - subarray.refreshDynamicEnergy * numActiveSubArrayPerColumn * numActiveSubArrayPerRow;

    routingLeakage = leakage - subarray.leakage * numColumnSubArray * numRowSubArray;

    /* For non-Htree bank, each layer contains an exact copy of this bank. */
    if (initialized & !invalid && stackedDieCount > 1) {
        leakage *= stackedDieCount;

        double tsvReadRampInput;
        double tsvWriteRampInput;

        // Normally senseAmpMuxLev2 is the last driver from SubArray
        // or mux from global sense amp if used
        //tsvReadRampInput = subarray.mat.senseAmpMuxLev2.rampOutput;
        tsvReadRampInput = 1e20;

        // Bank is the end unit for NVSIM, so we assume something external
        // is fully driving the input data values
        tsvWriteRampInput = infinite_ramp; 

        // Add TSV energy ~ Assume outside of bank area
        // Use comparator for tag read ramp input with internal sensing
        tsvArray.CalculateLatencyAndPower(tsvReadRampInput, tsvWriteRampInput); 

        //int numControlBits = (int)(log2((double)stackedDieCount + 0.1));
        int numControlBits = stackedDieCount;
        int numAddressBits = (int)(log2((double)capacity / blockSize / associativity / stackedDieCount) + 0.1);
        int numDataBits = blockSize * 2; // Read and write TSVs

        // Fine-granularity has predecoders on logic layer
        if (partitionGranularity == 1) {
            numAddressBits = 0;
        }

        double redundancyFactor = inputParameter->tsvRedundancy;
        tsvArray.numTotalBits = (int)((double)(numControlBits + numAddressBits + numDataBits) * redundancyFactor);
        tsvArray.numAccessBits = (int)((double)(numControlBits + numAddressBits + blockSize) * redundancyFactor);
        tsvArray.numReadBits = (int)((double)(numControlBits + numAddressBits) * redundancyFactor);
        tsvArray.numDataBits = (int)((double)(blockSize) * redundancyFactor);

        // Always assume worst case going to furthest die
        readLatency += (stackedDieCount-1) * tsvArray.readLatency
                     + (stackedDieCount-1) * tsvArray.writeLatency;
        writeLatency += (stackedDieCount-1) * tsvArray.writeLatency; 
        resetLatency += (stackedDieCount-1) * tsvArray.writeLatency; 
        setLatency += (stackedDieCount-1) * tsvArray.writeLatency; 
        refreshLatency += (stackedDieCount-1) * tsvArray.writeLatency;

        // Also assume worst energy
        readDynamicEnergy += tsvArray.numReadBits * (stackedDieCount-1) * tsvArray.writeDynamicEnergy + tsvArray.numDataBits * tsvArray.readDynamicEnergy * (stackedDieCount-1);
        writeDynamicEnergy += tsvArray.numAccessBits * (stackedDieCount-1) * tsvArray.writeDynamicEnergy;
        resetDynamicEnergy += tsvArray.numAccessBits * (stackedDieCount-1) * tsvArray.resetDynamicEnergy;
        setDynamicEnergy += tsvArray.numAccessBits * (stackedDieCount-1) * tsvArray.setDynamicEnergy;
        refreshDynamicEnergy += tsvArray.numReadBits * (stackedDieCount-1) * tsvArray.writeDynamicEnergy;

        leakage += tsvArray.numTotalBits * (stackedDieCount-1) * tsvArray.leakage;
    }

    if (cell->memCellType == eDRAM || cell->memCellType == gcDRAM) {
        if (refreshLatency > cell->retentionTime) {
            invalid = true;
        }
    }
}

BankWithoutHtree & BankWithoutHtree::operator=(const BankWithoutHtree &rhs) {
	Bank::operator=(rhs);
	numAddressBit = rhs.numAddressBit;
	numAddressBitRouteToSubArray = rhs.numAddressBitRouteToSubArray;
	numDataBitRouteToSubArray = rhs.numDataBitRouteToSubArray;
	return *this;
}

