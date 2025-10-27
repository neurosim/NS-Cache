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


#ifndef MAT_H_
#define MAT_H_

#include "FunctionUnit.h"
#include "Mat.h"
#include "PredecodeBlock.h"
#include "typedef.h"
#include "Comparator.h"
#include "TSV.h"

class SubArray: public FunctionUnit {
public:
	SubArray();
	virtual ~SubArray();

	/* Functions */
	void PrintProperty();
	void Initialize(int _numRowMat, int _numColumnMat, int _numAddressBit, long _numDataBit,
			int _numWay, int _numRowPerSet, bool _split, int _numActiveMatPerRow, int _numActiveMatPerColumn,
			int _muxSenseAmp, bool _internalSenseAmp, int _muxOutputLev1, int _muxOutputLev2,
			BufferDesignTarget _areaOptimizationLevel, MemoryType _memoryType, int _stackedDieCount,
            int _partitionGranularity, int monolithicStackCount);
	void CalculateArea();
	void CalculateRC();
	void CalculateLatency(double _rampInput);
	void CalculatePower();
	SubArray & operator=(const SubArray &);

	/* Properties */
	bool initialized;	/* Initialization flag */
	bool invalid;		/* Indicate that the current configuration is not valid, pass down to all the sub-components */
	bool internalSenseAmp;
	int numRowMat;		/* Number of mat rows in a subarray */
	int numColumnMat;	/* Number of mat columns in a subarray */
	int numAddressBit;		/* Number of subarray address bits */
	long numDataBit;		/* Number of subarray data bits */
	int numWay;				/* Number of cache ways distributed to this subarray, non-cache it is 1 */
	int numRowPerSet;		/* For cache design, the number of wordlines which a set is partitioned into */
	bool split;			/* Whether the row decoder is at the middle of mats */
	int numActiveMatPerRow;	/* For different access types */
	int numActiveMatPerColumn;	/* For different access types */
	int muxSenseAmp;	/* How many bitlines connect to one sense amplifier */
	int muxOutputLev1;	/* How many sense amplifiers connect to one output bit, level-1 */
	int muxOutputLev2;	/* How many sense amplifiers connect to one output bit, level-2 */
	BufferDesignTarget areaOptimizationLevel;
	MemoryType memoryType;
    int stackedDieCount;
    int partitionGranularity;

    int totalPredecoderOutputBits;

	double predecoderLatency;	/* The maximum latency of all the predecoder blocks, Unit: s */
    double areaAllLogicBlocks;

	Mat mat;
	PredecodeBlock rowPredecoderBlock1;
	PredecodeBlock rowPredecoderBlock2;
	PredecodeBlock bitlineMuxPredecoderBlock1;
	PredecodeBlock bitlineMuxPredecoderBlock2;
	PredecodeBlock senseAmpMuxLev1PredecoderBlock1;
	PredecodeBlock senseAmpMuxLev1PredecoderBlock2;
	PredecodeBlock senseAmpMuxLev2PredecoderBlock1;
	PredecodeBlock senseAmpMuxLev2PredecoderBlock2;

	Comparator comparator;

    TSV tsvArray;
};

#endif /* MAT_H_ */
