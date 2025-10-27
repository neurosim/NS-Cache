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


#include "SubArray.h"
#include "formula.h"
#include "global.h"

SubArray::SubArray() {
	// TODO Auto-generated constructor stub
	initialized = false;
	invalid = false;
}

SubArray::~SubArray() {
	// TODO Auto-generated destructor stub
}

void SubArray::Initialize(int _numRowMat, int _numColumnMat, int _numAddressBit, long _numDataBit,
		int _numWay, int _numRowPerSet, bool _split, int _numActiveMatPerRow, int _numActiveMatPerColumn,
		int _muxSenseAmp, bool _internalSenseAmp, int _muxOutputLev1, int _muxOutputLev2,
		BufferDesignTarget _areaOptimizationLevel, MemoryType _memoryType, int _stackedDieCount,
        int _partitionGranularity, int monolithicStackCount) {
	if (initialized)
		cout << "[SubArray] Warning: Already initialized!" << endl;

	numRowMat = _numRowMat;
	numColumnMat = _numColumnMat;
	numAddressBit = _numAddressBit;
	numDataBit = _numDataBit;
	numWay = _numWay;
	numRowPerSet = _numRowPerSet;
	split = _split;
	internalSenseAmp = _internalSenseAmp;
	areaOptimizationLevel = _areaOptimizationLevel;
	memoryType =_memoryType;
    stackedDieCount = _stackedDieCount;
    partitionGranularity = _partitionGranularity;

	if (_numActiveMatPerRow > numColumnMat) {
		cout << "[SubArray] Warning: The number of active mat per row is larger than the number of mat per row!"  << endl;
		cout << _numActiveMatPerRow << " > " << numColumnMat << endl;
		numActiveMatPerRow = numColumnMat;
	} else {
		numActiveMatPerRow = _numActiveMatPerRow;
	}
	if (_numActiveMatPerColumn > numRowMat) {
		cout << "[SubArray] Warning: The number of active mat per column is larger than the number of mat per column!"  << endl;
		cout << _numActiveMatPerColumn << " > " << numRowMat << endl;
		numActiveMatPerColumn = numRowMat;
	} else {
		numActiveMatPerColumn = _numActiveMatPerColumn;
	}
	muxSenseAmp = _muxSenseAmp;
	muxOutputLev1 = _muxOutputLev1;
	muxOutputLev2 = _muxOutputLev2;

	long long numRow = 0;		/* Number of rows in a mat */
	long long numColumn = 0;	/* Number of columns in a mat */

	/* The number of address bits that are used to power gate inactive mats */
	int numAddressForGating = (int)(log2(numRowMat * numColumnMat / numActiveMatPerColumn / numActiveMatPerRow)+0.1);
	_numAddressBit -= numAddressForGating;	/* Only use the effective address bits in the following calculation */
	if (_numAddressBit <= 0) {
		/* too aggressive partitioning */
		invalid = true;
		initialized = true;
		return;
	}

	/* Determine the number of rows in a mat */
	numRow = 1 << _numAddressBit;
	if (memoryType == data)
		numRow *= numWay;	/* Only for cache design that partitions a set into multiple rows */
	numRow /= (muxSenseAmp * muxOutputLev1 * muxOutputLev2);	/* Distribute to column decoding */
	if (numRow == 0) {
		invalid = true;
		initialized = true;
		return;
	}

	numColumn = (long long)numDataBit / (numActiveMatPerRow * numActiveMatPerColumn);	/* Adjust the number of columns depending on the access types */
	if (numColumn == 0) {
		invalid = true;
		initialized = true;
		return;
	}

	numColumn *= muxSenseAmp * muxOutputLev1 * muxOutputLev2;
	if (memoryType == tag)
		numColumn *= numWay;

	mat.Initialize(numRow, numColumn, numRowPerSet > 1, true /* TO-DO: need to correct */,
			muxSenseAmp, internalSenseAmp, muxOutputLev1, muxOutputLev2, areaOptimizationLevel, monolithicStackCount);

	if (mat.invalid) {
		invalid = true;
		initialized = true;
		return;
	}
	mat.CalculateArea();	/* the area needs to be calculated during the initialization because the size dimension needs to be called by others */

	int numAddressRowPredecoderBlock1 = _numAddressBit - (int)(log2(muxSenseAmp * muxOutputLev1 * muxOutputLev2)+0.1);	/* The address bit on row decodeing */
	if (numAddressRowPredecoderBlock1 < 0) {
		invalid = true;
		initialized = true;
		return;
	}
	int numAddressRowPredecoderBlock2 = 0;
	if (numAddressRowPredecoderBlock1 > 3) {	/* Block 2 is needed */
		numAddressRowPredecoderBlock2 = numAddressRowPredecoderBlock1 / 2;
		numAddressRowPredecoderBlock1 = numAddressRowPredecoderBlock1 - numAddressRowPredecoderBlock2;
	}

    totalPredecoderOutputBits = 1 << numAddressRowPredecoderBlock1; 
    totalPredecoderOutputBits += 1 << numAddressRowPredecoderBlock2; 

	double capLoadRowPredecoder = mat.height * localWire->capWirePerUnit * numRowMat / 2
			+ mat.width * localWire->capWirePerUnit * numColumnMat / 2;	/* Assume the predecoder is at the center */
	rowPredecoderBlock1.Initialize(numAddressRowPredecoderBlock1, capLoadRowPredecoder, 0 /* TO-DO */);
	rowPredecoderBlock2.Initialize(numAddressRowPredecoderBlock2, capLoadRowPredecoder, 0 /* TO-DO */);

	double capLoadMuxPredecoder = MAX(0, mat.height * localWire->capWirePerUnit * (numRowMat - 2) / 2)
			+ MAX(0, mat.width * localWire->capWirePerUnit * (numColumnMat - 2) / 2);
	int numAddressBitlineMuxPredecoderBlock1 = (int)(log2(muxSenseAmp) + 0.1);
	int numAddressBitlineMuxPredecoderBlock2 = 0;
	if (numAddressBitlineMuxPredecoderBlock1 > 3) {		/* Block 2 is needed */
		numAddressBitlineMuxPredecoderBlock2 = numAddressBitlineMuxPredecoderBlock1 / 2;
		numAddressBitlineMuxPredecoderBlock1 = numAddressBitlineMuxPredecoderBlock1 - numAddressBitlineMuxPredecoderBlock2;
	}
	bitlineMuxPredecoderBlock1.Initialize(numAddressBitlineMuxPredecoderBlock1, capLoadMuxPredecoder, 0 /* TO-DO */);
	bitlineMuxPredecoderBlock2.Initialize(numAddressBitlineMuxPredecoderBlock2, capLoadMuxPredecoder, 0 /* TO-DO */);

    totalPredecoderOutputBits += 1 << numAddressBitlineMuxPredecoderBlock1; 
    totalPredecoderOutputBits += 1 << numAddressBitlineMuxPredecoderBlock2; 

	int numAddressSenseAmpMuxLev1PredecoderBlock1 = (int)(log2(muxOutputLev1) + 0.1);
	int numAddressSenseAmpMuxLev1PredecoderBlock2 = 0;
	if (numAddressSenseAmpMuxLev1PredecoderBlock1 > 3) { /* Block 2 is needed */
		numAddressSenseAmpMuxLev1PredecoderBlock2 = numAddressSenseAmpMuxLev1PredecoderBlock1 / 2;
		numAddressSenseAmpMuxLev1PredecoderBlock1 = numAddressSenseAmpMuxLev1PredecoderBlock1 - numAddressSenseAmpMuxLev1PredecoderBlock2;
	}
	senseAmpMuxLev1PredecoderBlock1.Initialize(numAddressSenseAmpMuxLev1PredecoderBlock1, capLoadMuxPredecoder, 0 /* TO-DO */);
	senseAmpMuxLev1PredecoderBlock2.Initialize(numAddressSenseAmpMuxLev1PredecoderBlock2, capLoadMuxPredecoder, 0 /* TO-DO */);

    totalPredecoderOutputBits += 1 << numAddressSenseAmpMuxLev1PredecoderBlock1; 
    totalPredecoderOutputBits += 1 << numAddressSenseAmpMuxLev1PredecoderBlock2; 

	int numAddressSenseAmpMuxLev2PredecoderBlock1 = (int)(log2(muxOutputLev2) + 0.1);
	int numAddressSenseAmpMuxLev2PredecoderBlock2 = 0;
	if (numAddressSenseAmpMuxLev2PredecoderBlock1 > 3) { /* Block 2 is needed */
		numAddressSenseAmpMuxLev2PredecoderBlock2 = numAddressSenseAmpMuxLev2PredecoderBlock1 / 2;
		numAddressSenseAmpMuxLev2PredecoderBlock1 = numAddressSenseAmpMuxLev2PredecoderBlock1 - numAddressSenseAmpMuxLev2PredecoderBlock2;
	}
	senseAmpMuxLev2PredecoderBlock1.Initialize(numAddressSenseAmpMuxLev2PredecoderBlock1, capLoadMuxPredecoder, 0 /* TO-DO */);
	senseAmpMuxLev2PredecoderBlock2.Initialize(numAddressSenseAmpMuxLev2PredecoderBlock2, capLoadMuxPredecoder, 0 /* TO-DO */);

    totalPredecoderOutputBits += 1 << numAddressSenseAmpMuxLev2PredecoderBlock1; 
    totalPredecoderOutputBits += 1 << numAddressSenseAmpMuxLev2PredecoderBlock2; 

	if (memoryType == tag && internalSenseAmp) {
		comparator.Initialize(numDataBit, 0 /*TO-DO: need to fix */);
	}

    /* Initialize TSV connections. */
    if (stackedDieCount > 1 && partitionGranularity != 0) {
        TSV_type tsv_type = tech->WireTypeToTSVType(inputParameter->maxLocalWireType);
        tsvArray.Initialize(tsv_type);
    }

	initialized = true;
}

void SubArray::CalculateArea() {
	if (!initialized) {
		cout << "[SubArray] Error: Require initialization first!" << endl;
	} else if (invalid) {
		height = width = area = invalid_value;
	} else {
		/* mat CalculateArea() is already called during the initialization */
		rowPredecoderBlock1.CalculateArea();
		rowPredecoderBlock2.CalculateArea();
		bitlineMuxPredecoderBlock1.CalculateArea();
		bitlineMuxPredecoderBlock2.CalculateArea();
		senseAmpMuxLev1PredecoderBlock1.CalculateArea();
		senseAmpMuxLev1PredecoderBlock2.CalculateArea();
		senseAmpMuxLev2PredecoderBlock1.CalculateArea();
		senseAmpMuxLev2PredecoderBlock2.CalculateArea();

		double areaAllPredecoderBlocks = rowPredecoderBlock1.area + rowPredecoderBlock2.area
				+ bitlineMuxPredecoderBlock1.area + bitlineMuxPredecoderBlock2.area
				+ senseAmpMuxLev1PredecoderBlock1.area + senseAmpMuxLev1PredecoderBlock2.area
				+ senseAmpMuxLev2PredecoderBlock1.area + senseAmpMuxLev2PredecoderBlock2.area;
		width = mat.width * numColumnMat;
		height = mat.height * numRowMat;

        areaAllLogicBlocks = areaAllPredecoderBlocks;

        /* For any partition granularity besides coarse grained, predecoders go on logic layer. */
        if (stackedDieCount > 1 && partitionGranularity == 1) {
            /* Add TSV area for predecoders. */
            tsvArray.CalculateArea();
            double redundancyFactor = inputParameter->tsvRedundancy;
            double areaTSV = tsvArray.area * totalPredecoderOutputBits
                             * redundancyFactor;
            tsvArray.numTotalBits = (int)((double)(totalPredecoderOutputBits * redundancyFactor) + 0.1);
            tsvArray.numAccessBits = tsvArray.numTotalBits;

            /* Area of logic layer is computed during result output. */
            if (width > height)
                width += sqrt(areaTSV);
            else
                height += sqrt(areaTSV);
        } else {
            /* Add the predecoders' area */
            if (width > height)
                width += sqrt(areaAllPredecoderBlocks); // we don't want to have too much white space here.
            else
                height += sqrt(areaAllPredecoderBlocks);
        }

		if (memoryType == tag && internalSenseAmp) {
			comparator.CalculateArea();
            areaAllLogicBlocks += comparator.area;
            // TSVs for comparator are added above in previous conditional
            if (stackedDieCount <= 1 || partitionGranularity != 1) {
                height += numWay * comparator.area / width;
            }
		}

		area = height * width;
	}
}

void SubArray::CalculateRC() {
	if (!initialized) {
		cout << "[SubArray] Error: Require initialization first!" << endl;
	} else if (!invalid){
		/* mat does not have CalculateRC() function, since it is integrated as a part of initialization */
		rowPredecoderBlock1.CalculateRC();
		rowPredecoderBlock2.CalculateRC();
		bitlineMuxPredecoderBlock1.CalculateRC();
		bitlineMuxPredecoderBlock2.CalculateRC();
		senseAmpMuxLev1PredecoderBlock1.CalculateRC();
		senseAmpMuxLev1PredecoderBlock2.CalculateRC();
		senseAmpMuxLev2PredecoderBlock1.CalculateRC();
		senseAmpMuxLev2PredecoderBlock2.CalculateRC();
		if (memoryType == tag && internalSenseAmp) {
			comparator.CalculateRC();
		}
	}
}

void SubArray::CalculateLatency(double _rampInput) {
	if (!initialized) {
		cout << "[SubArray] Error: Require initialization first!" << endl;
	} else if (invalid) {
		readLatency = writeLatency = invalid_value;
	} else {
		/* Calculate the predecoder blocks latency */
		rowPredecoderBlock1.CalculateLatency(_rampInput);
		rowPredecoderBlock2.CalculateLatency(_rampInput);
		bitlineMuxPredecoderBlock1.CalculateLatency(_rampInput);
		bitlineMuxPredecoderBlock2.CalculateLatency(_rampInput);
		senseAmpMuxLev1PredecoderBlock1.CalculateLatency(_rampInput);
		senseAmpMuxLev1PredecoderBlock2.CalculateLatency(_rampInput);
		senseAmpMuxLev2PredecoderBlock1.CalculateLatency(_rampInput);
		senseAmpMuxLev2PredecoderBlock2.CalculateLatency(_rampInput);

		double rowPredecoderLatency = MAX(rowPredecoderBlock1.readLatency, rowPredecoderBlock2.readLatency);
		double bitlineMuxPredecoderLatency = MAX(bitlineMuxPredecoderBlock1.readLatency,
				bitlineMuxPredecoderBlock2.readLatency);
		double senseAmpMuxLev1PredecoderLatency = MAX(senseAmpMuxLev1PredecoderBlock1.readLatency,
				senseAmpMuxLev1PredecoderBlock2.readLatency);
		double senseAmpMuxLev2PredecoderLatency = MAX(senseAmpMuxLev2PredecoderBlock1.readLatency,
				senseAmpMuxLev2PredecoderBlock2.readLatency);
		predecoderLatency = MAX(MAX(rowPredecoderLatency, bitlineMuxPredecoderLatency),
				MAX(senseAmpMuxLev1PredecoderLatency, senseAmpMuxLev2PredecoderLatency));

        if (stackedDieCount > 1 && partitionGranularity != 0) {
            /* Add TSV latency here -- Once for address, once for data. */
            double tsvReadRampInput;
            double tsvWriteRampInput;

            // Normally senseAmpMuxLev2 is the last driver from SubArray
            //tsvReadRampInput = subarray.mat.senseAmpMuxLev2.rampOutput;
            tsvReadRampInput = 1e20;

            // Write TSVs should be driven by predecoders -- Use the min for worst case
            tsvWriteRampInput = infinite_ramp; 

            // Add TSV energy ~ Assume outside of bank area
            // Use comparator for tag read ramp input with internal sensing
            tsvArray.CalculateLatencyAndPower(tsvReadRampInput, tsvWriteRampInput); 

            // Address TSV latency
            predecoderLatency += (stackedDieCount-1) * tsvArray.writeLatency;
        }

		/* Caluclate mat latency */
		mat.CalculateLatency(MIN(rowPredecoderBlock1.rampOutput, rowPredecoderBlock2.rampOutput));

		/* Add them together */
		readLatency = predecoderLatency + mat.readLatency;
		writeLatency = predecoderLatency + mat.writeLatency;
		/* for RESET and SET only */
		resetLatency = predecoderLatency + mat.resetLatency;
		setLatency = predecoderLatency + mat.setLatency;
        /* Valid for DRAM and eDRAM only. */
        refreshLatency = predecoderLatency + mat.refreshLatency;
        refreshLatency *= numColumnMat; // TOTAL refresh time for all mats

		if (memoryType == tag && internalSenseAmp) {
			comparator.CalculateLatency(_rampInput);
			readLatency += comparator.readLatency;
		}
	}
}

void SubArray::CalculatePower() {
	if (!initialized) {
		cout << "[SubArray] Error: Require initialization first!" << endl;
	} else if (invalid) {
		readDynamicEnergy = writeDynamicEnergy = leakage = invalid_value;
	} else {
		rowPredecoderBlock1.CalculatePower();
		rowPredecoderBlock2.CalculatePower();
		bitlineMuxPredecoderBlock1.CalculatePower();
		bitlineMuxPredecoderBlock2.CalculatePower();
		senseAmpMuxLev1PredecoderBlock1.CalculatePower();
		senseAmpMuxLev1PredecoderBlock2.CalculatePower();
		senseAmpMuxLev2PredecoderBlock1.CalculatePower();
		senseAmpMuxLev2PredecoderBlock2.CalculatePower();
		mat.CalculatePower();

		readDynamicEnergy = rowPredecoderBlock1.readDynamicEnergy + rowPredecoderBlock2.readDynamicEnergy
				+ bitlineMuxPredecoderBlock1.readDynamicEnergy + bitlineMuxPredecoderBlock2.readDynamicEnergy
				+ senseAmpMuxLev1PredecoderBlock1.readDynamicEnergy + senseAmpMuxLev1PredecoderBlock2.readDynamicEnergy
				+ senseAmpMuxLev2PredecoderBlock1.readDynamicEnergy + senseAmpMuxLev2PredecoderBlock2.readDynamicEnergy;
		writeDynamicEnergy = rowPredecoderBlock1.writeDynamicEnergy + rowPredecoderBlock2.writeDynamicEnergy
				+ bitlineMuxPredecoderBlock1.writeDynamicEnergy + bitlineMuxPredecoderBlock2.writeDynamicEnergy
				+ senseAmpMuxLev1PredecoderBlock1.writeDynamicEnergy + senseAmpMuxLev1PredecoderBlock2.writeDynamicEnergy
				+ senseAmpMuxLev2PredecoderBlock1.writeDynamicEnergy + senseAmpMuxLev2PredecoderBlock2.writeDynamicEnergy;
        /* Assume the predecoder bits are broadcast, so we don't need to multiply by total mats / active. */
        refreshDynamicEnergy = rowPredecoderBlock1.readDynamicEnergy + rowPredecoderBlock2.readDynamicEnergy;
        refreshDynamicEnergy *= mat.numRow * numRowMat; // Total predecoder energy for all REFs
		leakage = rowPredecoderBlock1.leakage + rowPredecoderBlock2.leakage
				+ bitlineMuxPredecoderBlock1.leakage + bitlineMuxPredecoderBlock2.leakage
				+ senseAmpMuxLev1PredecoderBlock1.leakage + senseAmpMuxLev1PredecoderBlock2.leakage
				+ senseAmpMuxLev2PredecoderBlock1.leakage + senseAmpMuxLev2PredecoderBlock2.leakage;
		readDynamicEnergy += mat.readDynamicEnergy * numActiveMatPerRow * numActiveMatPerColumn;
        /* This is now the total refresh energy for this SubArray. */
        refreshDynamicEnergy += mat.refreshDynamicEnergy * numRowMat * numColumnMat;
		/* energy consumption on cells */
		cellReadEnergy = mat.cellReadEnergy * numActiveMatPerRow * numActiveMatPerColumn;
		cellSetEnergy = mat.cellSetEnergy * numActiveMatPerRow * numActiveMatPerColumn;
		cellResetEnergy = mat.cellResetEnergy * numActiveMatPerRow * numActiveMatPerColumn;
		/* for RESET and SET only */
		resetDynamicEnergy = writeDynamicEnergy + mat.resetDynamicEnergy * numActiveMatPerRow * numActiveMatPerColumn;
		setDynamicEnergy = writeDynamicEnergy + mat.setDynamicEnergy * numActiveMatPerRow * numActiveMatPerColumn;
		/* total write energy */
		writeDynamicEnergy += mat.writeDynamicEnergy * numActiveMatPerRow * numActiveMatPerColumn;
		leakage += mat.leakage * numRowMat * numColumnMat;

        if (stackedDieCount > 1 && partitionGranularity != 0) {
            // Add address TSV energy
            readDynamicEnergy += (stackedDieCount-1) * totalPredecoderOutputBits * tsvArray.readDynamicEnergy;
            writeDynamicEnergy += (stackedDieCount-1) * totalPredecoderOutputBits * tsvArray.writeDynamicEnergy;
            resetDynamicEnergy += (stackedDieCount-1) * totalPredecoderOutputBits * tsvArray.resetDynamicEnergy;
            setDynamicEnergy += (stackedDieCount-1) * totalPredecoderOutputBits * tsvArray.setDynamicEnergy;
            refreshDynamicEnergy += (stackedDieCount-1) * totalPredecoderOutputBits * tsvArray.readDynamicEnergy;

            leakage += tsvArray.numTotalBits * (stackedDieCount-1) * tsvArray.leakage;
        }

		if (memoryType == tag && internalSenseAmp) {
			comparator.CalculatePower();
			readDynamicEnergy += comparator.readDynamicEnergy * numWay;
			writeDynamicEnergy += comparator.writeDynamicEnergy * numWay;
			leakage += comparator.leakage * numWay;
		}

	}
}

void SubArray::PrintProperty() {
	cout << "SubArray Properties:" << endl;
	FunctionUnit::PrintProperty();
}


SubArray & SubArray::operator=(const SubArray &rhs) {
	//cout << "[PROGRESS] Line 397 :: SubArray.cc" << endl;
	height = rhs.height;
	width = rhs.width;
	area = rhs.area;
	readLatency = rhs.readLatency;
	writeLatency = rhs.writeLatency;
    refreshLatency = rhs.refreshLatency;
	readDynamicEnergy = rhs.readDynamicEnergy;
	writeDynamicEnergy = rhs.writeDynamicEnergy;
	resetLatency = rhs.resetLatency;
	setLatency = rhs.setLatency;
	resetDynamicEnergy = rhs.resetDynamicEnergy;
	setDynamicEnergy = rhs.setDynamicEnergy;
    refreshDynamicEnergy = rhs.refreshDynamicEnergy;
	cellReadEnergy = rhs.cellReadEnergy;
	cellSetEnergy = rhs.cellSetEnergy;
	cellResetEnergy = rhs.cellResetEnergy;
	leakage = rhs.leakage;
	initialized = rhs.initialized;
	invalid = rhs.invalid;
	numRowMat = rhs.numRowMat;
	numColumnMat = rhs.numColumnMat;
	numAddressBit = rhs.numAddressBit;
	numDataBit = rhs.numDataBit;
	numWay = rhs.numWay;
	numRowPerSet = rhs.numRowPerSet;
	split = rhs.split;
	internalSenseAmp = rhs.internalSenseAmp;
	numActiveMatPerRow = rhs.numActiveMatPerRow;
	numActiveMatPerColumn = rhs.numActiveMatPerColumn;
	muxSenseAmp = rhs.muxSenseAmp;
	muxOutputLev1 = rhs.muxOutputLev1;
	muxOutputLev2 = rhs.muxOutputLev2;
	areaOptimizationLevel = rhs.areaOptimizationLevel;
	memoryType = rhs.memoryType;
	predecoderLatency = rhs.predecoderLatency;
    areaAllLogicBlocks = rhs.areaAllLogicBlocks;

	mat = rhs.mat;
	rowPredecoderBlock1 = rhs.rowPredecoderBlock1;
	rowPredecoderBlock2 = rhs.rowPredecoderBlock2;
	bitlineMuxPredecoderBlock1 = rhs.bitlineMuxPredecoderBlock1;
	bitlineMuxPredecoderBlock2 = rhs.bitlineMuxPredecoderBlock2;
	senseAmpMuxLev1PredecoderBlock1 = rhs.senseAmpMuxLev1PredecoderBlock1;
	senseAmpMuxLev1PredecoderBlock2 = rhs.senseAmpMuxLev1PredecoderBlock2;
	senseAmpMuxLev2PredecoderBlock1 = rhs.senseAmpMuxLev2PredecoderBlock1;
	senseAmpMuxLev2PredecoderBlock2 = rhs.senseAmpMuxLev2PredecoderBlock2;
	if (memoryType == tag && internalSenseAmp)
		comparator = rhs.comparator;

    tsvArray = rhs.tsvArray;

	return *this;
}
