// This file contains code from NVSim, (c) 2012-2013,  Pennsylvania State University 
//and Hewlett-Packard Company. See LICENSE_NVSim file in the top-level directory.
//No part of DESTINY Project, including this file, may be copied,
//modified, propagated, or distributed except according to the terms
//contained in the LICENSE file.


#ifndef MACROS_H_
#define MACROS_H_


#define INITIAL_BASIC_WIRE { \
	WireType basicWireType; \
	WireRepeaterType basicWireRepeaterType; \
	bool isBasicLowSwing; \
	if (inputParameter->minLocalWireType == inputParameter->maxLocalWireType) \
		basicWireType = (WireType)inputParameter->minLocalWireType; \
	else \
		basicWireType = local_aggressive; \
	if (inputParameter->minLocalWireRepeaterType == inputParameter->maxLocalWireRepeaterType) \
		basicWireRepeaterType = (WireRepeaterType)inputParameter->minLocalWireRepeaterType; \
	else \
		basicWireRepeaterType = repeated_none; \
	if (inputParameter->minIsLocalWireLowSwing == inputParameter->maxIsLocalWireLowSwing) \
		isBasicLowSwing = inputParameter->minIsLocalWireLowSwing; \
	else \
		isBasicLowSwing = false; \
	localWire->Initialize(inputParameter->processNode, basicWireType, basicWireRepeaterType, inputParameter->temperature, isBasicLowSwing); \
	if (inputParameter->minGlobalWireType == inputParameter->maxGlobalWireType) \
		basicWireType = (WireType)inputParameter->minGlobalWireType; \
	else \
		basicWireType = global_aggressive; \
	if (inputParameter->minGlobalWireRepeaterType == inputParameter->maxGlobalWireRepeaterType) \
		basicWireRepeaterType = (WireRepeaterType)inputParameter->minGlobalWireRepeaterType; \
	else \
		basicWireRepeaterType = repeated_none; \
	if (inputParameter->minIsGlobalWireLowSwing == inputParameter->maxIsGlobalWireLowSwing) \
		isBasicLowSwing = inputParameter->minIsGlobalWireLowSwing; \
	else \
		isBasicLowSwing = false; \
	globalWire->Initialize(inputParameter->processNode, basicWireType, basicWireRepeaterType, inputParameter->temperature, isBasicLowSwing); \
}



#define REFINE_LOCAL_WIRE_FORLOOP \
	for (localWireType = inputParameter->minLocalWireType; localWireType <= inputParameter->maxLocalWireType; localWireType++) \
	for (localWireRepeaterType = inputParameter->minLocalWireRepeaterType; localWireRepeaterType <= inputParameter->maxLocalWireRepeaterType; localWireRepeaterType++) \
	for (isLocalWireLowSwing = inputParameter->minIsLocalWireLowSwing; isLocalWireLowSwing <= inputParameter->maxIsLocalWireLowSwing; isLocalWireLowSwing++) \
	if ((WireRepeaterType)localWireRepeaterType == repeated_none || (bool)isLocalWireLowSwing == false)


#define REFINE_GLOBAL_WIRE_FORLOOP \
	for (globalWireType = inputParameter->minGlobalWireType; globalWireType <= inputParameter->maxGlobalWireType; globalWireType++) \
	for (globalWireRepeaterType = inputParameter->minGlobalWireRepeaterType; globalWireRepeaterType <= inputParameter->maxGlobalWireRepeaterType; globalWireRepeaterType++) \
	for (isGlobalWireLowSwing = inputParameter->minIsGlobalWireLowSwing; isGlobalWireLowSwing <= inputParameter->maxIsGlobalWireLowSwing; isGlobalWireLowSwing++) \
	if ((WireRepeaterType)globalWireRepeaterType == repeated_none || (bool)isGlobalWireLowSwing == false)




#define LOAD_GLOBAL_WIRE(oldResult) { \
	globalWire->Initialize(inputParameter->processNode, (oldResult).globalWire->wireType, (oldResult).globalWire->wireRepeaterType, \
			inputParameter->temperature, (oldResult).globalWire->isLowSwing); \
}


#define LOAD_LOCAL_WIRE(oldResult) \
	localWire->Initialize(inputParameter->processNode, (oldResult).localWire->wireType, (oldResult).localWire->wireRepeaterType, \
			inputParameter->temperature, (oldResult).localWire->isLowSwing);



#define TRY_AND_UPDATE(oldResult, memoryType) { \
	if (inputParameter->routingMode == h_tree) \
		trialBank = new BankWithHtree(); \
	else \
		trialBank = new BankWithoutHtree(); \
	trialBank->Initialize((oldResult).bank->numRowSubArray, (oldResult).bank->numColumnSubArray, (oldResult).bank->capacity, (oldResult).bank->blockSize, (oldResult).bank->associativity, \
				(oldResult).bank->numRowPerSet, (oldResult).bank->numActiveSubArrayPerRow, (oldResult).bank->numActiveSubArrayPerColumn, (oldResult).bank->muxSenseAmp, \
				inputParameter->internalSensing, (oldResult).bank->muxOutputLev1, (oldResult).bank->muxOutputLev2, (oldResult).bank->numRowMat, (oldResult).bank->numColumnMat, \
				(oldResult).bank->numActiveMatPerRow, (oldResult).bank->numActiveMatPerColumn, (oldResult).bank->areaOptimizationLevel, (memoryType), (oldResult).bank->stackedDieCount, (oldResult).bank->partitionGranularity, inputParameter->monolithicStackCount); \
	trialBank->CalculateArea(); \
	trialBank->CalculateRC(); \
	trialBank->CalculateLatencyAndPower(); \
	*(tempResult.bank) = *trialBank; \
	*(tempResult.localWire) = *localWire; \
	*(tempResult.globalWire) = *globalWire; \
	oldResult.compareAndUpdate(tempResult); \
	delete trialBank; \
}



#define BIGFOR \
	for (numRowSubArray = inputParameter->minNumRowSubArray; numRowSubArray <= inputParameter->maxNumRowSubArray; numRowSubArray *= 2) \
	for (numColumnSubArray = inputParameter->minNumColumnSubArray; numColumnSubArray <= inputParameter->maxNumColumnSubArray; numColumnSubArray *= 2) \
    for (stackedDieCount = inputParameter->minStackLayer; stackedDieCount <= inputParameter->maxStackLayer; stackedDieCount *= 2) \
	for (numActiveSubArrayPerRow = MIN(numColumnSubArray, inputParameter->minNumActiveSubArrayPerRow); numActiveSubArrayPerRow <= MIN(numColumnSubArray, inputParameter->maxNumActiveSubArrayPerRow); numActiveSubArrayPerRow *= 2) \
	for (numActiveSubArrayPerColumn = MIN(numRowSubArray, inputParameter->minNumActiveSubArrayPerColumn); numActiveSubArrayPerColumn <= MIN(numRowSubArray, inputParameter->maxNumActiveSubArrayPerColumn); numActiveSubArrayPerColumn *= 2) \
	for (numRowMat = inputParameter->minNumRowMat; numRowMat <= inputParameter->maxNumRowMat; numRowMat *= 2) \
	for (numColumnMat = inputParameter->minNumColumnMat; numColumnMat <= inputParameter->maxNumColumnMat; numColumnMat *= 2) \
	for (numActiveMatPerRow = MIN(numColumnMat, inputParameter->minNumActiveMatPerRow); numActiveMatPerRow <= MIN(numColumnMat, inputParameter->maxNumActiveMatPerRow); numActiveMatPerRow *=2) \
	for (numActiveMatPerColumn = MIN(numRowMat, inputParameter->minNumActiveMatPerColumn); numActiveMatPerColumn <= MIN(numRowMat, inputParameter->maxNumActiveMatPerColumn); numActiveMatPerColumn *= 2) \
	for (muxSenseAmp = inputParameter->minMuxSenseAmp; muxSenseAmp <= inputParameter->maxMuxSenseAmp; muxSenseAmp *= 2) \
	for (muxOutputLev1 = inputParameter->minMuxOutputLev1; muxOutputLev1 <= inputParameter->maxMuxOutputLev1; muxOutputLev1 *= 2) \
	for (muxOutputLev2 = inputParameter->minMuxOutputLev2; muxOutputLev2 <= inputParameter->maxMuxOutputLev2; muxOutputLev2 *= 2) \
	for (numRowPerSet = inputParameter->minNumRowPerSet; numRowPerSet <= MIN(inputParameter->maxNumRowPerSet, inputParameter->associativity); numRowPerSet *= 2) \
	for (areaOptimizationLevel = inputParameter->minAreaOptimizationLevel; areaOptimizationLevel <= inputParameter->maxAreaOptimizationLevel; areaOptimizationLevel++)



#define CALCULATE(bank, memoryType) { \
	if (inputParameter->routingMode == h_tree) \
		(bank) = new BankWithHtree(); \
	else \
		(bank) = new BankWithoutHtree(); \
	(bank)->Initialize(numRowSubArray, numColumnSubArray, capacity, blockSize, associativity, \
				numRowPerSet, numActiveSubArrayPerRow, numActiveSubArrayPerColumn, muxSenseAmp, \
				inputParameter->internalSensing, muxOutputLev1, muxOutputLev2, numRowMat, numColumnMat, \
				numActiveMatPerRow, numActiveMatPerColumn, (BufferDesignTarget)areaOptimizationLevel, \
                (memoryType), stackedDieCount, partitionGranularity, inputParameter->monolithicStackCount); \
	(bank)->CalculateArea(); \
	(bank)->CalculateRC(); \
	(bank)->CalculateLatencyAndPower(); \
}


#define UPDATE_BEST_DATA { \
	*(tempResult.bank) = *dataBank; \
	*(tempResult.localWire) = *localWire; \
	*(tempResult.globalWire) = *globalWire; \
	for (int i = 0; i < (int)full_exploration; i++) \
		bestDataResults[i].compareAndUpdate(tempResult); \
}


#define VERIFY_DATA_CAPACITY { \
	if ((long long)dataBank->subarray.mat.numColumn * dataBank->subarray.mat.numRow * dataBank->numColumnSubArray * \
			dataBank->numRowSubArray * dataBank->numColumnMat * dataBank->numRowMat * stackedDieCount != capacity) { \
				cout << "1 Bank = " << dataBank->numRowSubArray << "x" << dataBank->numColumnSubArray << " SubArrays" << endl; \
				cout << "Activation - " << dataBank->numActiveSubArrayPerColumn << "x" << dataBank->numActiveSubArrayPerRow << " SubArrays" << endl; \
				cout << "1 SubArray  = " << dataBank->numRowMat << "x" << dataBank->numColumnMat << " Mats" << endl; \
				cout << "Activation - " << dataBank->numActiveMatPerColumn << "x" << dataBank->numActiveMatPerRow << " Mats" << endl; \
				cout << "Mux Degree - " << dataBank->muxSenseAmp << " x " << dataBank->muxOutputLev1 << " x " << dataBank->muxOutputLev2 << endl; \
				cout << "ERROR: DATA capacity violation. Shouldn't happen" << endl; \
                cout << "Saw " << ((long long)dataBank->subarray.mat.numColumn * dataBank->subarray.mat.numRow * dataBank->numColumnSubArray * dataBank->numRowSubArray * dataBank->numColumnMat * dataBank->numRowMat * stackedDieCount) << endl; \
                cout << "Expected " << capacity << endl; \
				exit(-1); \
			} \
}


#define UPDATE_BEST_TAG { \
	*(tempResult.bank) = *tagBank; \
	*(tempResult.localWire) = *localWire; \
	*(tempResult.globalWire) = *globalWire; \
	for (int i = 0; i < (int)full_exploration; i++) \
		bestTagResults[i].compareAndUpdate(tempResult); \
}


#define VERIFY_TAG_CAPACITY { \
	if ((long long)tagBank->subarray.mat.numColumn * tagBank->subarray.mat.numRow * tagBank->numColumnSubArray * \
			tagBank->numRowSubArray * tagBank->numColumnMat * tagBank->numRowMat * stackedDieCount != capacity) { \
				cout << "1 Bank = " << tagBank->numRowSubArray << "x" << tagBank->numColumnSubArray << " SubArrays" << endl; \
				cout << "Activation - " << tagBank->numActiveSubArrayPerColumn << "x" << tagBank->numActiveSubArrayPerRow << " SubArrays" << endl; \
				cout << "1 SubArray  = " << tagBank->numRowMat << "x" << tagBank->numColumnMat << " Mats" << endl; \
				cout << "Activation - " << tagBank->numActiveMatPerColumn << "x" << tagBank->numActiveMatPerRow << " Mats" << endl; \
				cout << "Mux Degree - " << tagBank->muxSenseAmp << " x " << tagBank->muxOutputLev1 << " x " << tagBank->muxOutputLev2 << endl; \
				cout << "ERROR: DATA capacity violation. Shouldn't happen" << endl; \
                cout << "Saw " << ((long long)tagBank->subarray.mat.numColumn * tagBank->subarray.mat.numRow * tagBank->numColumnSubArray * tagBank->numRowSubArray * tagBank->numColumnMat * tagBank->numRowMat * stackedDieCount) << endl; \
                cout << "Expected " << capacity << endl; \
				exit(-1); \
			} \
}


#define REDUCE_SEARCH_SIZE { \
	inputParameter->minNumRowSubArray = 1; \
	inputParameter->maxNumRowSubArray = 64; \
	inputParameter->minNumColumnSubArray = 1; \
	inputParameter->maxNumColumnSubArray = 64; \
	inputParameter->minNumActiveSubArrayPerRow = 1; \
	inputParameter->maxNumActiveSubArrayPerRow = inputParameter->maxNumColumnSubArray; \
	inputParameter->minNumActiveSubArrayPerColumn = 1; \
	inputParameter->maxNumActiveSubArrayPerColumn = inputParameter->maxNumRowSubArray; \
	inputParameter->minNumRowMat = 1; \
	inputParameter->maxNumRowMat = 2; \
	inputParameter->minNumColumnMat = 1; \
	inputParameter->maxNumColumnMat = 2; \
	inputParameter->minNumActiveMatPerRow = 1; \
	inputParameter->maxNumActiveMatPerRow = inputParameter->maxNumColumnMat; \
	inputParameter->minNumActiveMatPerColumn = 1; \
	inputParameter->maxNumActiveMatPerColumn = inputParameter->maxNumRowMat; \
	inputParameter->minMuxSenseAmp = 1; \
	inputParameter->maxMuxSenseAmp = 64; \
	inputParameter->minMuxOutputLev1 = 1; \
	inputParameter->maxMuxOutputLev1 = 64; \
	inputParameter->minMuxOutputLev2 = 1; \
	inputParameter->maxMuxOutputLev2 = 64; \
	inputParameter->minNumRowPerSet = 1; \
	inputParameter->maxNumRowPerSet = 1; \
	inputParameter->minAreaOptimizationLevel = latency_first; \
	inputParameter->maxAreaOptimizationLevel = area_first;	\
	inputParameter->minLocalWireType = local_aggressive; \
	inputParameter->maxLocalWireType = local_conservative; \
	inputParameter->minGlobalWireType = global_aggressive; \
	inputParameter->maxGlobalWireType = global_conservative; \
	inputParameter->minLocalWireRepeaterType = repeated_none; \
	inputParameter->maxLocalWireRepeaterType = repeated_opt; \
	inputParameter->minGlobalWireRepeaterType = repeated_none; \
	inputParameter->maxGlobalWireRepeaterType = repeated_opt; \
	inputParameter->minIsLocalWireLowSwing = false; \
	inputParameter->maxIsLocalWireLowSwing = true; \
	inputParameter->minIsGlobalWireLowSwing = false; \
	inputParameter->maxIsGlobalWireLowSwing = true; \
}


#define REDUCE_SEARCH_SIZE_CONSTRAINED { \
    if (inputParameter->maxNumRowSubArray != inputParameter->minNumRowSubArray) { \
	inputParameter->minNumRowSubArray = 1; \
	inputParameter->maxNumRowSubArray = 64; } \
    if (inputParameter->maxNumColumnSubArray != inputParameter->minNumColumnSubArray) { \
	inputParameter->minNumColumnSubArray = 1; \
	inputParameter->maxNumColumnSubArray = 64; } \
    if (inputParameter->maxNumActiveSubArrayPerRow != inputParameter->minNumActiveSubArrayPerRow) { \
	inputParameter->minNumActiveSubArrayPerRow = 1; \
	inputParameter->maxNumActiveSubArrayPerRow = inputParameter->maxNumColumnSubArray; } \
    if (inputParameter->maxNumActiveSubArrayPerColumn != inputParameter->minNumActiveSubArrayPerColumn) { \
	inputParameter->minNumActiveSubArrayPerColumn = 1; \
	inputParameter->maxNumActiveSubArrayPerColumn = inputParameter->maxNumRowSubArray; } \
    if (inputParameter->maxNumRowMat != inputParameter->minNumRowMat) { \
	inputParameter->minNumRowMat = 1; \
	inputParameter->maxNumRowMat = 2; } \
    if (inputParameter->maxNumColumnMat != inputParameter->minNumColumnMat) { \
	inputParameter->minNumColumnMat = 1; \
	inputParameter->maxNumColumnMat = 2; } \
    if (inputParameter->maxNumActiveMatPerRow != inputParameter->minNumActiveMatPerRow) { \
	inputParameter->minNumActiveMatPerRow = 1; \
	inputParameter->maxNumActiveMatPerRow = inputParameter->maxNumColumnMat; } \
    if (inputParameter->maxNumActiveMatPerColumn != inputParameter->minNumActiveMatPerColumn) { \
	inputParameter->minNumActiveMatPerColumn = 1; \
	inputParameter->maxNumActiveMatPerColumn = inputParameter->maxNumRowMat; } \
    if (inputParameter->maxMuxSenseAmp != inputParameter->minMuxSenseAmp) { \
	inputParameter->minMuxSenseAmp = 1; \
	inputParameter->maxMuxSenseAmp = 64; } \
    if (inputParameter->maxMuxOutputLev1 != inputParameter->minMuxOutputLev1) { \
	inputParameter->minMuxOutputLev1 = 1; \
	inputParameter->maxMuxOutputLev1 = 64; } \
    if (inputParameter->maxMuxOutputLev2 != inputParameter->minMuxOutputLev2) { \
	inputParameter->minMuxOutputLev2 = 1; \
	inputParameter->maxMuxOutputLev2 = 64; } \
	inputParameter->minNumRowPerSet = 1; \
	inputParameter->maxNumRowPerSet = 1; \
	inputParameter->minAreaOptimizationLevel = latency_first; \
	inputParameter->maxAreaOptimizationLevel = area_first;	\
	inputParameter->minLocalWireType = local_aggressive; \
	inputParameter->maxLocalWireType = local_conservative; \
	inputParameter->minGlobalWireType = global_aggressive; \
	inputParameter->maxGlobalWireType = global_conservative; \
	inputParameter->minLocalWireRepeaterType = repeated_none; \
	inputParameter->maxLocalWireRepeaterType = repeated_opt; \
	inputParameter->minGlobalWireRepeaterType = repeated_none; \
	inputParameter->maxGlobalWireRepeaterType = repeated_opt; \
	inputParameter->minIsLocalWireLowSwing = false; \
	inputParameter->maxIsLocalWireLowSwing = true; \
	inputParameter->minIsGlobalWireLowSwing = false; \
	inputParameter->maxIsGlobalWireLowSwing = true; \
}


#define RESTORE_SEARCH_SIZE { \
	inputParameter->minNumRowSubArray = 1; \
	inputParameter->maxNumRowSubArray = 512; \
	inputParameter->minNumColumnSubArray = 1; \
	inputParameter->maxNumColumnSubArray = 512; \
	inputParameter->minNumActiveSubArrayPerRow = 1; \
	inputParameter->maxNumActiveSubArrayPerRow = inputParameter->maxNumColumnSubArray; \
	inputParameter->minNumActiveSubArrayPerColumn = 1; \
	inputParameter->maxNumActiveSubArrayPerColumn = inputParameter->maxNumRowSubArray; \
	inputParameter->minNumRowMat = 1; \
	inputParameter->maxNumRowMat = 2; \
	inputParameter->minNumColumnMat = 1; \
	inputParameter->maxNumColumnMat = 2; \
	inputParameter->minNumActiveMatPerRow = 1; \
	inputParameter->maxNumActiveMatPerRow = inputParameter->maxNumColumnMat; \
	inputParameter->minNumActiveMatPerColumn = 1; \
	inputParameter->maxNumActiveMatPerColumn = inputParameter->maxNumRowMat; \
	inputParameter->minMuxSenseAmp = 1; \
	inputParameter->maxMuxSenseAmp = 256; \
	inputParameter->minMuxOutputLev1 = 1; \
	inputParameter->maxMuxOutputLev1 = 256; \
	inputParameter->minMuxOutputLev2 = 1; \
	inputParameter->maxMuxOutputLev2 = 256; \
	inputParameter->minNumRowPerSet = 1; \
	inputParameter->maxNumRowPerSet = inputParameter->associativity; \
	inputParameter->minAreaOptimizationLevel = latency_first; \
	inputParameter->maxAreaOptimizationLevel = area_first; \
	inputParameter->minLocalWireType = local_aggressive; \
	inputParameter->maxLocalWireType = semi_conservative; \
	inputParameter->minGlobalWireType = semi_aggressive; \
	inputParameter->maxGlobalWireType = global_conservative; \
	inputParameter->minLocalWireRepeaterType = repeated_none; \
	inputParameter->maxLocalWireRepeaterType = repeated_50;		/* The limit is repeated_50 */ \
	inputParameter->minGlobalWireRepeaterType = repeated_none; \
	inputParameter->maxGlobalWireRepeaterType = repeated_50;	/* The limit is repeated_50 */ \
	inputParameter->minIsLocalWireLowSwing = false; \
	inputParameter->maxIsLocalWireLowSwing = true; \
	inputParameter->minIsGlobalWireLowSwing = false; \
	inputParameter->maxIsGlobalWireLowSwing = true; \
}


#define APPLY_LIMIT(result) { \
	(result).reset(); \
	(result).limitReadLatency = allowedDataReadLatency; \
	(result).limitWriteLatency = allowedDataWriteLatency; \
	(result).limitReadDynamicEnergy = allowedDataReadDynamicEnergy; \
	(result).limitWriteDynamicEnergy = allowedDataWriteDynamicEnergy; \
	(result).limitReadEdp = allowedDataReadEdp; \
	(result).limitWriteEdp = allowedDataWriteEdp; \
	(result).limitArea = allowedDataArea; \
	(result).limitLeakage = allowedDataLeakage; \
}


#define OUTPUT_TO_FILE { \
	if (inputParameter->designTarget == cache) { \
		for (int i = 0; i < (int)full_exploration; i++) \
			tempResult.printAsCacheToCsvFile(bestTagResults[i], inputParameter->cacheAccessMode, outputFile); \
	} else { \
		tempResult.printToCsvFile(outputFile); \
		outputFile << endl; \
	} \
}

#define TO_GENERAL(x) \
	((x) < 1e-15 ? (x) * 1e18 : (x) < 1e-12 ? (x) * 1e15 : (x) < 1e-9 ? (x) * 1e12 : (x) < 1e-6 ? (x) * 1e9 : (x) < 1e-3 ? (x) * 1e6 : (x) < 1 ? (x) * 1e3 : (x) < 1e3 ? (x) * 1e6 : (x) < 1e6 ? (x) * 1e9 : (x)) \
	<< \
	((x) < 1e-15 ? "a" : (x) < 1e-12 ? "f" : (x) < 1e-9 ? "p" : (x) < 1e-6 ? "n" : (x) < 1e-3 ? "u" : (x) < 1 ? "m" : (x) < 1e3 ? "units" :  (x) < 1e6 ? "K" : "G")

#define TO_SECOND(x) \
	((x) < 1e-9 ? (x) * 1e12 : (x) < 1e-6 ? (x) * 1e9 : (x) < 1e-3 ? (x) * 1e6 : (x) < 1 ? (x) * 1e3 : (x)) \
	<< \
	((x) < 1e-9 ? "ps" : (x) < 1e-6 ? "ns" : (x) < 1e-3 ? "us" : (x) < 1 ? "ms" : "s")

#define TO_BPS(x) \
	((x) < 1e3 ? (x) : (x) < 1e6 ? (x) / 1e3 : (x) < 1e9 ? (x) / 1e6 : (x) < 1e12 ? (x) / 1e9 : (x) / 1e12) \
	<< \
	((x) < 1e3 ? "B/s" : (x) < 1e6 ? "KB/s" : (x) < 1e9 ? "MB/s" : (x) < 1e12 ? "GB/s" : "TB/s")

#define TO_JOULE(x) \
	((x) < 1e-9 ? (x) * 1e12 : (x) < 1e-6 ? (x) * 1e9 : (x) < 1e-3 ? (x) * 1e6 : (x) < 1 ? (x) * 1e3 : (x)) \
	<< \
	((x) < 1e-9 ? "pJ" : (x) < 1e-6 ? "nJ" : (x) < 1e-3 ? "uJ" : (x) < 1 ? "mJ" : "J")

#define TO_WATT(x) \
	((x) < 1e-9 ? (x) * 1e12 : (x) < 1e-6 ? (x) * 1e9 : (x) < 1e-3 ? (x) * 1e6 : (x) < 1 ? (x) * 1e3 : (x)) \
	<< \
	((x) < 1e-9 ? "pW" : (x) < 1e-6 ? "nW" : (x) < 1e-3 ? "uW" : (x) < 1 ? "mW" : "W")

#define TO_METER(x) \
	((x) < 1e-9 ? (x) * 1e12 : (x) < 1e-6 ? (x) * 1e9 : (x) < 1e-3 ? (x) * 1e6 : (x) < 1 ? (x) * 1e3 : (x)) \
	<< \
	((x) < 1e-9 ? "pm" : (x) < 1e-6 ? "nm" : (x) < 1e-3 ? "um" : (x) < 1 ? "mm" : "m")

#define TO_SQM(x) \
	((x) < 1e-12 ? (x) * 1e18 : (x) < 1e-6 ? (x) * 1e12 : (x) < 1 ? (x) * 1e6 : (x)) \
	<< \
	((x) < 1e-12 ? "nm^2" : (x) < 1e-6 ? "um^2" : (x) < 1 ? "mm^2" : "m^2")

#endif /* MACROS_H_ */
