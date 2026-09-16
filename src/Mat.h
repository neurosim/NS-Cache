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


#ifndef SUBARRAY_H_
#define SUBARRAY_H_

#include "FunctionUnit.h"
#include "RowDecoder.h"
#include "Precharger.h"
#include "SenseAmp.h"
#include "Mux.h"
#include "LevelShifter.h"
#include "TSV.h"
#include "typedef.h"
#include "AOSFETCompactModel.h"
#include <cstdint>

struct DRAMTimingResult {
	double accessLatency = 0;       /* Time to data, Unit: s */
	double restoreDelay = 0;        /* Post-read restoration, Unit: s */
	double readCycleLatency = 0;    /* Access plus restoration, Unit: s */
	double writeBitlineDelay = 0;   /* Loaded write-path settling, Unit: s */
};

struct GcDRAMPowerResult {
	double readBitlineAccessEnergy = 0;
	double writeBitlineAccessEnergy = 0;
	double writeDriverEnergy = 0;
	double aosLeakageUpperBound = 0;
};

enum class M3DDominantTier {
	none,
	logic,
	memory
};

struct M3DLayoutResult {
	std::uint64_t mivsPerTier = 0;
	std::uint64_t totalMivCount = 0;
	double totalMivArea = 0;
	double peripheralLogicArea = 0;
	double finalLogicLayerArea = 0;
	double perTierMemoryArea = 0;
	double projectedArea = 0;
	M3DDominantTier dominantTier = M3DDominantTier::none;
};

class Mat: public FunctionUnit {
public:
	Mat();
	virtual ~Mat();

	/* Functions */
	void PrintProperty();
	void Initialize(long long _numRow, long long _numColumn, bool _multipleRowPerSet, bool _split,
			int _muxSenseAmp, bool _internalSenseAmp, int _muxOutputLev1, int _muxOutputLev2,
			BufferDesignTarget _areaOptimizationLevel, int _num3DLevels);
	void CalculateArea();
	//void CalculateRC();
	void CalculateLatency(double _rampInput);
	void CalculatePower();
	void CalculateRepeater(int numCol);
	Mat & operator=(const Mat &) = default;

	/* Properties */
	bool initialized = false;	/* Initialization flag */
	bool invalid = false;		/* Indicate that the current configuration is not valid, pass down to all the sub-components */
	bool internalSenseAmp = false; /* Indicate whether sense amp is within mat */
	long long numRow = 0;			/* Number of rows */
	long long numColumn = 0;		/* Number of columns */
	bool multipleRowPerSet = false;		/* For cache design, whether a set is partitioned into multiple wordlines */
	bool split = false;			/* Whether the row decoder is at the middle of mats */
	int muxSenseAmp = 0;	/* How many bitlines connect to one sense amplifier */
	int muxOutputLev1 = 0;	/* How many sense amplifiers connect to one output bit, level-1 */
	int muxOutputLev2 = 0;	/* How many sense amplifiers connect to one output bit, level-2 */
	BufferDesignTarget areaOptimizationLevel = latency_first;
	TSV_type tsvType = Fine;
    int num3DLevels = 0; /* Number of monolithic 3D levels in the mat. */

	double capWordlineRead = 0;	/* Wordline capacitance, Gain Cell, Unit: F */
	double capBitlineRead = 0;	/* Bitline capacitance, Gain Cell, Unit: F */
	bool voltageSense = false;	/* Whether the sense amplifier is voltage-sensing */
	double senseVoltage = 0;/* Minimum sensible voltage */
	double voltagePrecharge = 0;
	long long numSenseAmp = 0;	/* Number of sense amplifiers */
	double lenWordline = 0;	/* Length of wordlines, Unit: m */
	double lenBitline = 0;	/* Length of bitlines, Unit: m */
	double capWordline = 0;	/* Wordline capacitance, Unit: F */
	double capBitline = 0;	/* Bitline capacitance, Unit: F */
	double capPlateline = 0;
	double resWordline = 0;	/* Wordline resistance, Unit: ohm */
	double resBitline = 0;	/* Bitline resistance, Unit: ohm */
	double resPlateline = 0;
	double resReadWordline = 0;	/* Wordline resistance, Unit: ohm */
	double resWriteBitline = 0;	/* Bitline resistance, Unit: ohm */
	double resCellAccess = 0; /* Resistance of access device, Unit: ohm */
	double resCellAccessOff = 0;
	double capCellAccess = 0; /* Capacitance of access device, Unit: F */
	double resMemCellOff = 0;  /* HRS resistance, Unit: ohm */
	double resMemCellOn = 0;   /* LRS resistance, Unit: ohm */
	double voltageMemCellOff = 0; /* Voltage drop on HRS during read operation, Unit: V */
	double voltageMemCellOn = 0;   /* Voltage drop on LRS druing read operation, Unit: V */
	double resInSerialForSenseAmp = 0; /* Serial resistance of voltage-in voltage sensing as a voltage divider, Unit: ohm */
	double resEquivalentOn = 0;          /* resInSerialForSenseAmp in parallel with resMemCellOn, Unit: ohm */
	double resEquivalentOff = 0;          /* resInSerialForSenseAmp in parallel with resMemCellOn, Unit: ohm */
	double bitlineDelay = 0;	/* Bitline delay, Unit: s */
	double readBitlineDelay = 0;	/* gain cell read Bitline delay, Unit: s */
	double writeBitlineDelay = 0;	/* gain cell write Bitline delay, Unit: s */
	double chargeLatency = 0;	/* The bitline charge delay during write operations, Unit: s */
	double chargeReadLatency = 0;	/* The bitline charge delay during Read operations, Unit: s */
	double columnDecoderLatency = 0;	/* The worst-case mux latency, Unit: s */
	double bitlineDelayOn = 0;  /* Bitline delay of LRS, Unit: s */
	double bitlineDelayOff = 0; /* Bitline delay of HRS, Unit: s */
	double logicArea = 0;		/* Legacy alias for final M3D logic-layer area */
	double logicWidth = 0;
	double logicHeight = 0;
	double memoryArea = 0;		/* Legacy alias for per-tier memory area */
	double memoryWidth = 0;
	double memoryHeight = 0;
	double areaRatio = 0;
	int stackedMemTiers = 1;
	double resReadCellAccess = 0; /* Resistance of access device, Unit: ohm */ //gcDRAM bidir only
	double resReadCellAccessOff = 0;
	double capReadCellAccess = 0; /* Drain capacitance of access device, Unit: F */ //gcDRAM bidir only
	double capReadCellGate = 0;
	double resWriteCellAccess = 0; /* Resistance of access device, Unit: ohm */ //gcDRAM bidir only
	double resWriteCellAccessOff = 0;
	double capWriteCellAccess = 0; /* Drain capacitance of access device, Unit: F */ //gcDRAM bidir only
	double capWriteCellGate = 0;
	int largerLine = 0;

	DRAMTimingResult dramTiming;
	GcDRAMPowerResult gcDramPower;
	M3DLayoutResult m3d;
	double aosLeakageUpperBound = 0;
	AOSDeviceOperatingPoint aosAccessOperatingPoint{};
	AOSDeviceOperatingPoint aosReadOperatingPoint{};
	AOSDeviceOperatingPoint aosWriteOperatingPoint{};


	// 1.4 update : parameters for buffer insertion
	double widthInvN = 0, widthInvP = 0;
	double wInv = 0, hInv = 0, drivecapin = 0, drivecapout = 0, targetdriveres = 0;
	double sectionres = 0, sectioncap = 0, sectionresMux = 0, sectioncapMux = 0;
	double activityRowRead = 0, activityRowWrite = 0;		// Activity for # of rows
	double gateCapRep = 0;
	int numRepeaters = 0;
	double bufferSizeRatio = 1;

	RowDecoder	rowDecoder;
	RowDecoder	gcRowDecoder;
	RowDecoder	plateLineDecoder;
	RowDecoder	bitlineMuxDecoder;
	Mux			bitlineMux;
	RowDecoder	senseAmpMuxLev1Decoder;
	Mux			senseAmpMuxLev1;
	RowDecoder	senseAmpMuxLev2Decoder;
	Mux			senseAmpMuxLev2;
	Precharger	precharger;
	Precharger	writeDriver;
	SenseAmp	senseAmp;

	/* Monolithic 3D Update: Add MIV Grid for M3D connectivit*/
	TSV tsvArray;
};

#endif /* SUBARRAY_H_ */
