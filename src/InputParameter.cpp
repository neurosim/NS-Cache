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


#include "InputParameter.h"
#include "global.h"
#include "constant.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

InputParameter::InputParameter() {
	// TODO Auto-generated constructor stub
	designTarget = cache;
	optimizationTarget = read_latency_optimized;
	processNode = 90;
	maxDriverCurrent = 0;

	maxNmosSize = MAX_NMOS_SIZE;

	minNumRowSubArray = 1;
	maxNumRowSubArray = 512;
	minNumColumnSubArray = 1;
	maxNumColumnSubArray = 512;
    minStackLayer = 1;
    maxStackLayer = 16;
	minNumActiveSubArrayPerRow = 1;
	maxNumActiveSubArrayPerRow = maxNumColumnSubArray;
	minNumActiveSubArrayPerColumn = 1;
	maxNumActiveSubArrayPerColumn = maxNumRowSubArray;
	minNumRowMat = 4;
	maxNumRowMat = 4;
	minNumColumnMat = 4;
	maxNumColumnMat = 4;
	minNumActiveMatPerRow = 4;
	maxNumActiveMatPerRow = maxNumColumnMat;
	minNumActiveMatPerColumn = 4;
	maxNumActiveMatPerColumn = maxNumRowMat;
	minMuxSenseAmp = 1;
	maxMuxSenseAmp = 256;
	minMuxOutputLev1 = 1;
	maxMuxOutputLev1 = 256;
	minMuxOutputLev2 = 1;
	maxMuxOutputLev2 = 256;
	minNumRowPerSet = 1;
	maxNumRowPerSet = 256;
	minAreaOptimizationLevel = latency_first;
	maxAreaOptimizationLevel = area_first;
	minLocalWireType = local_aggressive;
	maxLocalWireType = local_conservative;
	minGlobalWireType = global_aggressive;
	maxGlobalWireType = global_conservative;
	minLocalWireRepeaterType = repeated_none;
	maxLocalWireRepeaterType = repeated_50;		/* The limit is repeated_50 */
	minGlobalWireRepeaterType = repeated_none;
	maxGlobalWireRepeaterType = repeated_50;	/* The limit is repeated_50 */
	minIsLocalWireLowSwing = false;
	maxIsLocalWireLowSwing = true;
	minIsGlobalWireLowSwing = false;
	maxIsGlobalWireLowSwing = true;
	/* relaxSRAMCell = true; */
	relaxSRAMCell = true;
	delaytolerance = 0.1;
	numRepeaters = 0;
	bufferSizeRatio = 1.0;
	addRepeaters = false;
	clockFreq = 3e9;
	quantize = false;

	associativity = 1;				/* Default value for non-cache design */
	routingMode = h_tree;
	internalSensing = true;

	useCactiAssumption = false;

	writeScheme = normal_write;
	cacheAccessMode = normal_access_mode;

	readLatencyConstraint = invalid_value;
	writeLatencyConstraint = invalid_value;
	readDynamicEnergyConstraint = invalid_value;
	writeDynamicEnergyConstraint = invalid_value;
	leakageConstraint = invalid_value;
	areaConstraint = invalid_value;
	readEdpConstraint = invalid_value;
	writeEdpConstraint = invalid_value;
	isConstraintApplied = false;
	isPruningEnabled = false;
	viewMatStats = false;
	monolithic3DMat = false;

	pageSize = 0;
	flashBlockSize = 0;

	outputFilePrefix = "output";	/* Default output file name */

    partitionGranularity = 0; /* 0: Coarse-Grained Bank Level (Just distribute subarrays evenly), 1: Fine-Grained Bank-Level (use N+1 layers with N layers of subarrays and one shared logic layer) */
    localTsvProjection = 1; /* 0: ITRS aggressive, 1: Industrial conservative */
    globalTsvProjection = 1; /* 0: ITRS aggressive, 1: Industrial conservative */
    tsvRedundancy = 1.5;

    monolithicStackCount = 1;
	maxMatLayers = 4;

    fileMemCell.clear();

    doublePrune = false;  // TODO
    printAllOptimals = false;
    allowDifferentTagTech = false;
	validated = true;

    printLevel = 1;
}

InputParameter::~InputParameter() {
	// TODO Auto-generated destructor stub
}

void InputParameter::ReadInputParameterFromFile(const std::string & inputFile) {
	FILE *fp = fopen(inputFile.c_str(), "r");
	char line[5000];
	char tmp[5000];

	if (!fp) {
		cout << inputFile << " cannot be found!\n";
		cout<<" This file may be present in \"config\" folder. If so, please run destiny from that folder, otherwise, change the file name to include folder location.\n";
		exit(-1);
	}

    forcedStackLayers = false;

	while (fscanf(fp, "%[^\n]\n", line) != EOF) {
		if (!strncmp("-DesignTarget", line, strlen("-DesignTarget"))) {
			sscanf(line, "-DesignTarget: %s", tmp);
			if (!strcmp(tmp, "cache"))
				designTarget = cache;
			else if (!strcmp(tmp, "RAM")) {
				designTarget = RAM_chip;
				minNumRowPerSet = 1;
				maxNumRowPerSet = 1;
			} else {
				designTarget = CAM_chip;
				minNumRowPerSet = 1;
				maxNumRowPerSet = 1;
			}
			continue;
		}

		if (!strncmp("-OptimizationTarget", line, strlen("-OptimizationTarget"))) {
			sscanf(line, "-OptimizationTarget: %s", tmp);
			if (!strcmp(tmp, "ReadLatency"))
				optimizationTarget = read_latency_optimized;
			else if (!strcmp(tmp, "WriteLatency"))
			optimizationTarget = write_latency_optimized;
			else if (!strcmp(tmp, "ReadDynamicEnergy"))
				optimizationTarget = read_energy_optimized;
			else if (!strcmp(tmp, "WriteDynamicEnergy"))
				optimizationTarget = write_energy_optimized;
			else if (!strcmp(tmp, "ReadEDP"))
				optimizationTarget = read_edp_optimized;
			else if (!strcmp(tmp, "WriteEDP"))
				optimizationTarget = write_edp_optimized;
			else if (!strcmp(tmp, "LeakagePower"))
				optimizationTarget = leakage_optimized;
			else if (!strcmp(tmp, "Area"))
				optimizationTarget = area_optimized;
			else
				optimizationTarget = full_exploration;
			continue;
		}

		if (!strncmp("-OutputFilePrefix", line, strlen("-OutputFilePrefix"))) {
			sscanf(line, "-OutputFilePrefix: %s", tmp);
			outputFilePrefix = (string)tmp;
			continue;
		}

		if (!strncmp("-ProcessNode", line, strlen("-ProcessNode"))) {
			sscanf(line, "-ProcessNode: %d", &processNode);
			continue;
		}
		if (!strncmp("-Capacity (B)", line, strlen("-Capacity (B)"))) {
			long cap;
			sscanf(line, "-Capacity (B): %ld", &cap);
			capacity = cap;
			continue;
		}
		if (!strncmp("-Capacity (KB)", line, strlen("-Capacity (KB)"))) {
			long cap;
			sscanf(line, "-Capacity (KB): %ld", &cap);
			capacity = cap * 1024;
			continue;
		}
		if (!strncmp("-Capacity (MB)", line, strlen("-Capacity (MB)"))) {
			long cap;
			sscanf(line, "-Capacity (MB): %ld", &cap);
			capacity = cap * 1024*1024;
			continue;
		}
		if (!strncmp("-WordWidth", line, strlen("-WordWidth"))) {
			sscanf(line, "-WordWidth (bit): %ld", &wordWidth);
			continue;
		}
		if (!strncmp("-Associativity", line, strlen("-Associativity"))) {
			sscanf(line, "-Associativity (for cache only): %d", &associativity);
			continue;
		}
		if (!strncmp("-Temperature", line, strlen("-Temperature"))) {
			sscanf(line, "-Temperature (K): %d", &temperature);
			continue;
		}
		if (!strncmp("-MaxDriverCurrent", line, strlen("-MaxDriverCurrent"))) {
			sscanf(line, "-MaxDriverCurrent (uA): %lf", &maxDriverCurrent);
			continue;
		}
		if (!strncmp("-DeviceRoadmap", line, strlen("-DeviceRoadmap"))) {
			sscanf(line, "-DeviceRoadmap: %s", tmp);
			if (!strcmp(tmp, "HP"))
				deviceRoadmap = HP;
			else if (!strcmp(tmp, "LSTP"))
				deviceRoadmap = LSTP;
			else
				deviceRoadmap = LOP;
			continue;
		}

		if (!strncmp("-WriteScheme", line, strlen("-WriteScheme"))) {
			sscanf(line, "-WriteScheme: %s", tmp);
			if (!strcmp(tmp, "SetBeforeReset"))
				writeScheme = set_before_reset;
			else if (!strcmp(tmp, "ResetBeforeSet"))
				writeScheme = reset_before_set;
			else if (!strcmp(tmp, "EraseBeforeSet"))
				writeScheme = erase_before_set;
			else if (!strcmp(tmp, "EraseBeforeReset"))
				writeScheme = erase_before_reset;
			else if (!strcmp(tmp, "WriteAndVerify"))
				writeScheme = write_and_verify;
			else
				writeScheme = normal_write;
			continue;
		}

		if (!strncmp("-CacheAccessMode", line, strlen("-CacheAccessMode"))) {
			sscanf(line, "-CacheAccessMode: %s", tmp);
			if (!strcmp(tmp, "Sequential"))
				cacheAccessMode = sequential_access_mode;
			else if (!strcmp(tmp, "Fast"))
				cacheAccessMode = fast_access_mode;
			else
				cacheAccessMode = normal_access_mode;
			continue;
		}

		if (!strncmp("-LocalWireType", line, strlen("-LocalWireType"))) {
			sscanf(line, "-LocalWireType: %s", tmp);
			if (!strcmp(tmp, "LocalAggressive")) {
				minLocalWireType = local_aggressive;
				maxLocalWireType = local_aggressive;
			} else if (!strcmp(tmp, "LocalConservative")) {
				minLocalWireType = local_conservative;
				maxLocalWireType = local_conservative;
			} else if (!strcmp(tmp, "SemiAggressive")) {
				minLocalWireType = semi_aggressive;
				maxLocalWireType = semi_aggressive;
			} else if (!strcmp(tmp, "SemiConservative")) {
				minLocalWireType = semi_conservative;
				maxLocalWireType = semi_conservative;
			} else if (!strcmp(tmp, "GlobalAggressive")) {
				minLocalWireType = global_aggressive;
				maxLocalWireType = global_aggressive;
			} else if (!strcmp(tmp, "GlobalConservative")) {
				minLocalWireType = global_conservative;
				maxLocalWireType = global_conservative;
			} else {	/* no supported yet */
				minLocalWireType = dram_wordline;
				maxLocalWireType = dram_wordline;
			}
			continue;
		}
		if (!strncmp("-LocalWireRepeaterType", line, strlen("-LocalWireRepeaterType"))) {
			sscanf(line, "-LocalWireRepeaterType: %s", tmp);
			if (!strcmp(tmp, "RepeatedOpt")) {
				minLocalWireRepeaterType = repeated_opt;
				maxLocalWireRepeaterType = repeated_opt;
			} else if (!strcmp(tmp, "Repeated5%Penalty")) {
				minLocalWireRepeaterType = repeated_5;
				maxLocalWireRepeaterType = repeated_5;
			} else if (!strcmp(tmp, "Repeated10%Penalty")) {
				minLocalWireRepeaterType = repeated_10;
				maxLocalWireRepeaterType = repeated_10;
			} else if (!strcmp(tmp, "Repeated20%Penalty")) {
				minLocalWireRepeaterType = repeated_20;
				maxLocalWireRepeaterType = repeated_20;
			} else if (!strcmp(tmp, "Repeated30%Penalty")) {
				minLocalWireRepeaterType = repeated_30;
				maxLocalWireRepeaterType = repeated_30;
			} else if (!strcmp(tmp, "Repeated40%Penalty")) {
				minLocalWireRepeaterType = repeated_40;
				maxLocalWireRepeaterType = repeated_40;
			} else if (!strcmp(tmp, "Repeated50%Penalty")) {
				minLocalWireRepeaterType = repeated_50;
				maxLocalWireRepeaterType = repeated_50;
			} else {
				minLocalWireRepeaterType = repeated_none;
				maxLocalWireRepeaterType = repeated_none;
			}
			continue;
		}
		if (!strncmp("-LocalWireUseLowSwing", line, strlen("-LocalWireUseLowSwing"))) {
			sscanf(line, "-LocalWireUseLowSwing: %s", tmp);
			if (!strcmp(tmp, "Yes")) {
				minIsLocalWireLowSwing = 1;
				maxIsLocalWireLowSwing = 1;
			} else {
				minIsLocalWireLowSwing = 0;
				maxIsLocalWireLowSwing = 0;
			}
			continue;
		}

		if (!strncmp("-GlobalWireType", line, strlen("-GlobalWireType"))) {
			sscanf(line, "-GlobalWireType: %s", tmp);
			if (!strcmp(tmp, "LocalAggressive")) {
				minGlobalWireType = local_aggressive;
				maxGlobalWireType = local_aggressive;
			} else if (!strcmp(tmp, "LocalConservative")) {
				minGlobalWireType = local_conservative;
				maxGlobalWireType = local_conservative;
			} else if (!strcmp(tmp, "SemiAggressive")) {
				minGlobalWireType = semi_aggressive;
				maxGlobalWireType = semi_aggressive;
			} else if (!strcmp(tmp, "SemiConservative")) {
				minGlobalWireType = semi_conservative;
				maxGlobalWireType = semi_conservative;
			} else if (!strcmp(tmp, "GlobalAggressive")) {
				minGlobalWireType = global_aggressive;
				maxGlobalWireType = global_aggressive;
			} else if (!strcmp(tmp, "GlobalConservative")) {
				minGlobalWireType = global_conservative;
				maxGlobalWireType = global_conservative;
			} else {	/* no supported yet */
				minGlobalWireType = dram_wordline;
				maxGlobalWireType = dram_wordline;
			}
			continue;
		}
		if (!strncmp("-GlobalWireRepeaterType", line, strlen("-GlobalWireRepeaterType"))) {
			sscanf(line, "-GlobalWireRepeaterType: %s", tmp);
			if (!strcmp(tmp, "RepeatedOpt")) {
				minGlobalWireRepeaterType = repeated_opt;
				maxGlobalWireRepeaterType = repeated_opt;
			} else if (!strcmp(tmp, "Repeated5%Penalty")) {
				minGlobalWireRepeaterType = repeated_5;
				maxGlobalWireRepeaterType = repeated_5;
			} else if (!strcmp(tmp, "Repeated10%Penalty")) {
				minGlobalWireRepeaterType = repeated_10;
				maxGlobalWireRepeaterType = repeated_10;
			} else if (!strcmp(tmp, "Repeated20%Penalty")) {
				minGlobalWireRepeaterType = repeated_20;
				maxGlobalWireRepeaterType = repeated_20;
			} else if (!strcmp(tmp, "Repeated30%Penalty")) {
				minGlobalWireRepeaterType = repeated_30;
				maxGlobalWireRepeaterType = repeated_30;
			} else if (!strcmp(tmp, "Repeated40%Penalty")) {
				minGlobalWireRepeaterType = repeated_40;
				maxGlobalWireRepeaterType = repeated_40;
			} else if (!strcmp(tmp, "Repeated50%Penalty")) {
				minGlobalWireRepeaterType = repeated_50;
				maxGlobalWireRepeaterType = repeated_50;
			} else {
				minGlobalWireRepeaterType = repeated_none;
				maxGlobalWireRepeaterType = repeated_none;
			}
			continue;
		}
		if (!strncmp("-GlobalWireUseLowSwing", line, strlen("-GlobalWireUseLowSwing"))) {
			sscanf(line, "-GlobalWireUseLowSwing: %s", tmp);
			if (!strcmp(tmp, "Yes")) {
				minIsGlobalWireLowSwing = 1;
				maxIsGlobalWireLowSwing = 1;
			} else {
				minIsGlobalWireLowSwing = 0;
				maxIsGlobalWireLowSwing = 0;
			}
			continue;
		}

		if (!strncmp("-Routing", line, strlen("-Routing"))) {
			sscanf(line, "-Routing: %s", tmp);
			if (!strcmp(tmp, "H-tree"))
				routingMode = h_tree;
			else
				routingMode = non_h_tree;
			continue;
		}

		if (!strncmp("-InternalSensing", line, strlen("-InternalSensing"))) {
			sscanf(line, "-InternalSensing: %s", tmp);
			if (!strcmp(tmp, "true"))
				internalSensing = true;
			else
				internalSensing = false;
			continue;
		}

		if (!strncmp("-MemoryCellInputFile", line, strlen("-MemoryCellInputFile"))) {
			sscanf(line, "-MemoryCellInputFile: %s", tmp);
			fileMemCell.push_back(string(tmp));
			continue;
		}

		if (!strncmp("-MaxNmosSize", line, strlen("-MaxNmosSize"))) {
			sscanf(line, "-MaxNmosSize (F): %lf", &maxNmosSize);
			continue;
		}

		if (!strncmp("-ForceBank3DA", line, strlen("-ForceBank3DA"))) {
			sscanf(line, "-ForceBank3DA (Total AxBxC): %dx%dx%d",
					&minNumRowSubArray, &minNumColumnSubArray, &minStackLayer);
			maxNumRowSubArray = minNumRowSubArray;
			maxNumColumnSubArray = minNumColumnSubArray;
            maxStackLayer = minStackLayer;
            cout << "Forcing bank 3d" << endl;
            if (forcedStackLayers) {
                cout << "Warning: Number of die stacked layers specified twice (by -ForceBank3D and -StackedDieCount!" << endl;
            }
            forcedStackLayers = true;
			continue;
		}

		if (!strncmp("-ForceBank3D", line, strlen("-ForceBank3D"))) {
			sscanf(line, "-ForceBank3D (Total AxBxC, Active DxE): %dx%dx%d, %dx%d",
					&minNumRowSubArray, &minNumColumnSubArray, &minStackLayer, &minNumActiveSubArrayPerColumn, &minNumActiveSubArrayPerRow);
			maxNumRowSubArray = minNumRowSubArray;
			maxNumColumnSubArray = minNumColumnSubArray;
            maxStackLayer = minStackLayer;
			maxNumActiveSubArrayPerColumn = minNumActiveSubArrayPerColumn;
			maxNumActiveSubArrayPerRow = minNumActiveSubArrayPerRow;
            if (forcedStackLayers) {
                cout << "Warning: Number of die stacked layers specified twice (by -ForceBank3D and -StackedDieCount!" << endl;
            }
            forcedStackLayers = true;
			continue;
		}

		if (!strncmp("-ForceBankA", line, strlen("-ForceBankA"))) {
			sscanf(line, "-ForceBankA (Total AxB): %dx%d",
					&minNumRowSubArray, &minNumColumnSubArray);
			maxNumRowSubArray = minNumRowSubArray;
			maxNumColumnSubArray = minNumColumnSubArray;
			continue;
		}

		if (!strncmp("-LimitMonolithicTier", line, strlen("-LimitMonolithicTier"))) {
			sscanf(line, "-LimitMonolithicTier (N): %d", &matLayerSet);
			maxMatLayers = matLayerSet;
			continue;
		}

		if (!strncmp("-ClockFrequency", line, strlen("-ClockFrequency"))) {
			sscanf(line, "-ClockFrequency: %d", &clockInput);
			clockFreq = clockInput;
			continue;
		}

		if (!strncmp("-ForceBank", line, strlen("-ForceBank"))) {
			sscanf(line, "-ForceBank (Total AxB, Active CxD): %dx%d, %dx%d",
					&minNumRowSubArray, &minNumColumnSubArray, &minNumActiveSubArrayPerColumn, &minNumActiveSubArrayPerRow);
			maxNumRowSubArray = minNumRowSubArray;
			maxNumColumnSubArray = minNumColumnSubArray;
			maxNumActiveSubArrayPerColumn = minNumActiveSubArrayPerColumn;
			maxNumActiveSubArrayPerRow = minNumActiveSubArrayPerRow;
			continue;
		}

		if (!strncmp("-ForceSubArrayA", line, strlen("-ForceSubArrayA"))) {
			sscanf(line, "-ForceSubArrayA (Total AxB): %dx%d",
					&minNumRowMat, &minNumColumnMat);
			maxNumRowMat = minNumRowMat;
			maxNumColumnMat = minNumColumnMat;
            cout << "Forcing SubArray " << endl;
			continue;
		}

		if (!strncmp("-ForceSubArray", line, strlen("-ForceSubArray"))) {
			sscanf(line, "-ForceSubArray (Total AxB, Active CxD): %dx%d, %dx%d",
					&minNumRowMat, &minNumColumnMat, &minNumActiveMatPerColumn, &minNumActiveMatPerRow);
			maxNumRowMat = minNumRowMat;
			maxNumColumnMat = minNumColumnMat;
			maxNumActiveMatPerColumn = minNumActiveMatPerColumn;
			maxNumActiveMatPerRow = minNumActiveMatPerRow;
			continue;
		}

		if (!strncmp("-ForceMuxSenseAmp", line, strlen("-ForceMuxSenseAmp"))) {
			sscanf(line, "-ForceMuxSenseAmp: %d", &minMuxSenseAmp);
			maxMuxSenseAmp = minMuxSenseAmp;
			continue;
		}

		if (!strncmp("-ForceMuxOutputLev1", line, strlen("-ForceMuxOutputLev1"))) {
			sscanf(line, "-ForceMuxOutputLev1: %d", &minMuxOutputLev1);
			maxMuxOutputLev1 = minMuxOutputLev1;
			continue;
		}

		if (!strncmp("-ForceMuxOutputLev2", line, strlen("-ForceMuxOutputLev2"))) {
			sscanf(line, "-ForceMuxOutputLev2: %d", &minMuxOutputLev2);
			maxMuxOutputLev2 = minMuxOutputLev2;
			continue;
		}

		if (!strncmp("-UseCactiAssumption", line, strlen("-UseCactiAssumption"))) {
			sscanf(line, "-UseCactiAssumption: %s", tmp);
			if (!strcmp(tmp, "Yes")) {
				useCactiAssumption = true;
				minNumActiveSubArrayPerRow = maxNumColumnSubArray;
				maxNumActiveSubArrayPerRow = maxNumColumnSubArray;
				minNumActiveSubArrayPerColumn = 1;
				maxNumActiveSubArrayPerColumn = 1;
				minNumRowMat = 2;
				maxNumRowMat = 2;
				minNumColumnMat = 2;
				maxNumColumnMat = 2;
				minNumActiveMatPerRow = 2;
				maxNumActiveMatPerRow = 2;
				minNumActiveMatPerColumn = 2;
				maxNumActiveMatPerColumn = 2;
			} else
				useCactiAssumption = false;
			continue;
		}

		if (!strncmp("-EnablePruning", line, strlen("-EnablePruning"))) {
			sscanf(line, "-EnablePruning: %s", tmp);
			if (!strcmp(tmp, "Yes"))
				isPruningEnabled = true;
			else
				isPruningEnabled = false;
			continue;
		}

		if (!strncmp("-BufferDesignOptimization", line, strlen("-BufferDesignOptimization"))) {
			sscanf(line, "-BufferDesignOptimization: %s", tmp);
			if (!strcmp(tmp, "latency")) {
				minAreaOptimizationLevel = 0;
				maxAreaOptimizationLevel = 0;
			} else if (!strcmp(tmp, "area")) {
				minAreaOptimizationLevel = 2;
				maxAreaOptimizationLevel = 2;
			} else {
				minAreaOptimizationLevel = 1;
				maxAreaOptimizationLevel = 1;
			}
		}

		if (!strncmp("-FlashPageSize", line, strlen("-FlashPageSize"))) {
			sscanf(line, "-FlashPageSize (Byte): %ld", &pageSize);
			pageSize *= 8;	/* Byte to bit */
			continue;
		}

		if (!strncmp("-FlashBlockSize", line, strlen("-FlashBlockSize"))) {
			sscanf(line, "-FlashBlockSize (KB): %ld", &flashBlockSize);
			flashBlockSize *= (8 * 1024);	/* KB to bit */
			continue;
		}

		if (!strncmp("-ApplyReadLatencyConstraint", line, strlen("-ApplyReadLatencyConstraint"))) {
			sscanf(line, "-ApplyReadLatencyConstraint: %lf", &readLatencyConstraint);
			isConstraintApplied = true;
			continue;
		}

		if (!strncmp("-ApplyWriteLatencyConstraint", line, strlen("-ApplyWriteLatencyConstraint"))) {
			sscanf(line, "-ApplyWriteLatencyConstraint: %lf", &writeLatencyConstraint);
			isConstraintApplied = true;
			continue;
		}

		if (!strncmp("-ApplyReadDynamicEnergyConstraint", line, strlen("-ApplyReadDynamicEnergyConstraint"))) {
			sscanf(line, "-ApplyReadDynamicEnergyConstraint: %lf", &readDynamicEnergyConstraint);
			isConstraintApplied = true;
			continue;
		}

		if (!strncmp("-ApplyWriteDynamicEnergyConstraint", line, strlen("-ApplyWriteDynamicEnergyConstraint"))) {
			sscanf(line, "-ApplyWriteDynamicEnergyConstraint: %lf", &writeDynamicEnergyConstraint);
			isConstraintApplied = true;
			continue;
		}

		if (!strncmp("-ApplyLeakageConstraint", line, strlen("-ApplyLeakageConstraint"))) {
			sscanf(line, "-ApplyLeakageConstraint: %lf", &leakageConstraint);
			isConstraintApplied = true;
			continue;
		}

		if (!strncmp("-ApplyAreaConstraint", line, strlen("-ApplyAreaConstraint"))) {
			sscanf(line, "-ApplyAreaConstraint: %lf", &areaConstraint);
			isConstraintApplied = true;
			continue;
		}

		if (!strncmp("-ApplyReadEdpConstraint", line, strlen("-ApplyReadEdpConstraint"))) {
			sscanf(line, "-ApplyReadEdpConstraint: %lf", &readEdpConstraint);
			isConstraintApplied = true;
			continue;
		}

		if (!strncmp("-ApplyWriteEdpConstraint", line, strlen("-ApplyWriteEdpConstraint"))) {
			sscanf(line, "-ApplyWriteEdpConstraint: %lf", &writeEdpConstraint);
			isConstraintApplied = true;
			continue;
		}

		if (!strncmp("-ViewMatStatistics", line, strlen("-ViewMatStatistics"))) {
			sscanf(line, "-ViewMatStatistics: %lf", tmp);
			viewMatStats = true;
			continue;
		}

		if (!strncmp("-ViewQuantization", line, strlen("-VieQuantization"))) {
			sscanf(line, "-ViewQuantization: %lf", tmp);
			quantize = true;
			continue;
		}

		if (!strncmp("-M3DMemory", line, strlen("-M3DMemory"))) {
			sscanf(line, "-M3DMemory: %lf", tmp);
			monolithic3DMat = true;
			continue;
		}

        if (!strncmp("-PartitionGranularity", line, strlen("-PartitionGranularity"))) {
            sscanf(line, "-PartitionGranularity: %d", &partitionGranularity);
            continue;
        }

        if (!strncmp("-LocalTSVProjection", line, strlen("-LocalTSVProjection"))) {
            sscanf(line, "-LocalTSVProjection: %d", &localTsvProjection);
            continue;
        }

        if (!strncmp("-GlobalTSVProjection", line, strlen("-GlobalTSVProjection"))) {
            sscanf(line, "-GlobalTSVProjection: %d", &globalTsvProjection);
            continue;
        }

        if (!strncmp("-TSVRedundancy", line, strlen("-TSVRedundancy"))) {
            sscanf(line, "-TSVRedundancy: %lf", &tsvRedundancy);
            continue;
        }

        if (!strncmp("-StackedDieCount", line, strlen("-StackedDieCount"))) {
            sscanf(line, "-StackedDieCount: %d", &minStackLayer);
            maxStackLayer = minStackLayer;
            if (forcedStackLayers) {
                cout << "Warning: Number of die stacked layers specified twice (by -ForceBank3D and -StackedDieCount!" << endl;
            }
            forcedStackLayers = true;
            continue;
        }

        if (!strncmp("-MonolithicStackCount", line, strlen("-MonolithicStackCount"))) {
            sscanf(line, "-MonolithicStackCount: %d", &monolithicStackCount);
            continue;
        }

        if (!strncmp("-PrintAllOptimals", line, strlen("-PrintAllOptimals"))) {
            sscanf(line, "-PrintAllOptimals: %s", tmp);
            if (!strcmp(tmp, "true"))
                printAllOptimals = true;
            else
                printAllOptimals = false;
            continue;
        }

        if (!strncmp("-AllowDifferentTagTech", line, strlen("-AllowDifferentTagTech"))) {
            sscanf(line, "-AllowDifferentTagTech: %s", tmp);
            if (!strcmp(tmp, "true"))
                allowDifferentTagTech = true;
            else
                allowDifferentTagTech = false;
            continue;
        }

        if (!strncmp("-PrintLevel", line, strlen("-PrintLevel"))) {
            sscanf(line, "-PrintLevel: %d", &printLevel);
        }
	}

	fclose(fp);
}

void InputParameter::PrintInputParameter() {
	cout << endl << "====================" << endl << "DESIGN SPECIFICATION" << endl << "====================" << endl;
	cout << "Design Target: ";
	switch (designTarget) {
	case cache:
		cout << "Cache" << endl;
		break;
	case RAM_chip:
		cout << "Random Access Memory" << endl;
		break;
	default:	/* CAM */
		cout << "Content Addressable Memory" << endl;
	}

	cout << "Capacity   : ";
	if (capacity < 1024 * 1024)
		cout << capacity / 1024 << "KB" << endl;
	else if (capacity < 1024 * 1024 * 1024)
		cout << capacity / 1024 / 1024 << "MB" << endl;
	else
		cout << capacity / 1024 / 1024 / 1024 << "GB" << endl;

	if (designTarget == cache) {
		cout << "Cache Line Size: " << wordWidth / 8 << "Bytes" << endl;
		cout << "Cache Associativity: " << associativity << " Ways" << endl;
	} else {
		cout << "Data Width : " << wordWidth << "Bits";
		if (wordWidth % 8 == 0)
			cout << " (" << wordWidth / 8 << "Bytes)" << endl;
		else
			cout << endl;
	}
	if (designTarget == RAM_chip && (cell->memCellType == SLCNAND || cell->memCellType == MLCNAND)) {
		cout << "Page Size  : " << pageSize / 8 << "Bytes" << endl;
		cout << "Block Size : " << flashBlockSize / 8 / 1024 << "KB" << endl;
	}
	// TO-DO: tedious work here!!!

	if (optimizationTarget == full_exploration) {
		cout << endl << "Full design space exploration ... might take hours and will produce a csv" << endl;
		cout << "If you are interested in optimizing for a single metric only, please change -OptimizationTarget in *cfg file, for example \n -OptimizationTarget: WriteEDP" << endl;
	} else {
		cout << endl << "Searching for the best solution that is optimized for ";
		switch (optimizationTarget) {
		case read_latency_optimized:
			cout << "read latency ..." << endl;
			break;
		case write_latency_optimized:
			cout << "write latency ..." << endl;
			break;
		case read_energy_optimized:
			cout << "read energy ..." << endl;
			break;
		case write_energy_optimized:
			cout << "write energy ..." << endl;
			break;
		case read_edp_optimized:
			cout << "read energy-delay-product ..." << endl;
			break;
		case write_edp_optimized:
			cout << "write energy-delay-product ..." << endl;
			break;
		case leakage_optimized:
			cout << "leakage power ..." << endl;
			break;
		default:	/* area */
			cout << "area ..." << endl;
		}
	}
}
