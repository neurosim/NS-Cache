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
	Mat & operator=(const Mat &);

	/* Properties */
	bool initialized;	/* Initialization flag */
	bool invalid;		/* Indicate that the current configuration is not valid, pass down to all the sub-components */
	bool internalSenseAmp; /* Indicate whether sense amp is within mat */
	long long numRow;			/* Number of rows */
	long long numColumn;		/* Number of columns */
	bool multipleRowPerSet;		/* For cache design, whether a set is partitioned into multiple wordlines */
	bool split;			/* Whether the row decoder is at the middle of mats */
	int muxSenseAmp;	/* How many bitlines connect to one sense amplifier */
	int muxOutputLev1;	/* How many sense amplifiers connect to one output bit, level-1 */
	int muxOutputLev2;	/* How many sense amplifiers connect to one output bit, level-2 */
	BufferDesignTarget areaOptimizationLevel;
	TSV_type tsvType;
    int num3DLevels; /* Number of monolithic 3D levels in the mat. */

	double capWordlineRead;	/* Wordline capacitance, Gain Cell, Unit: F */
	double capBitlineRead;	/* Bitline capacitance, Gain Cell, Unit: F */
	bool voltageSense;	/* Whether the sense amplifier is voltage-sensing */
	double senseVoltage;/* Minimum sensible voltage */
	double voltagePrecharge;
	long long numSenseAmp;	/* Number of sense amplifiers */
	double lenWordline;	/* Length of wordlines, Unit: m */
	double lenBitline;	/* Length of bitlines, Unit: m */
	double capWordline;	/* Wordline capacitance, Unit: F */
	double capBitline;	/* Bitline capacitance, Unit: F */
	double capPlateline;
	double resWordline;	/* Wordline resistance, Unit: ohm */
	double resBitline;	/* Bitline resistance, Unit: ohm */
	double resPlateline;
	double resReadWordline;	/* Wordline resistance, Unit: ohm */
	double resWriteBitline;	/* Bitline resistance, Unit: ohm */
	double resCellAccess; /* Resistance of access device, Unit: ohm */
	double capCellAccess; /* Capacitance of access device, Unit: ohm */
	double resMemCellOff;  /* HRS resistance, Unit: ohm */
	double resMemCellOn;   /* LRS resistance, Unit: ohm */
	double voltageMemCellOff; /* Voltage drop on HRS during read operation, Unit: V */
	double voltageMemCellOn;   /* Voltage drop on LRS druing read operation, Unit: V */
	double resInSerialForSenseAmp; /* Serial resistance of voltage-in voltage sensing as a voltage divider, Unit: ohm */
	double resEquivalentOn;          /* resInSerialForSenseAmp in parallel with resMemCellOn, Unit: ohm */
	double resEquivalentOff;          /* resInSerialForSenseAmp in parallel with resMemCellOn, Unit: ohm */
	double bitlineDelay;	/* Bitline delay, Unit: s */
	double readBitlineDelay;	/* gain cell read Bitline delay, Unit: s */
	double writeBitlineDelay;	/* gain cell write Bitline delay, Unit: s */
	double chargeLatency;	/* The bitline charge delay during write operations, Unit: s */
	double chargeReadLatency;	/* The bitline charge delay during Read operations, Unit: s */
	double columnDecoderLatency;	/* The worst-case mux latency, Unit: s */
	double bitlineDelayOn;  /* Bitline delay of LRS, Unit: s */
	double bitlineDelayOff; /* Bitline delay of HRS, Unit: s */
	double logicArea;		/* Seperation of FEOL logic from BEOL Mat */
	double logicWidth;		/* Seperation of FEOL logic from BEOL Mat */
	double logicHeight;		/* Seperation of FEOL logic from BEOL Mat */
	double memoryArea;		/* Seperation of FEOL logic from BEOL Mat */
	double memoryWidth;		/* Seperation of FEOL logic from BEOL Mat */
	double memoryHeight;
	double areaRatio;		/* Seperation of FEOL logic from BEOL Mat */
	int stackedMemTiers;
	double resReadCellAccess; /* Resistance of access device, Unit: ohm */ //gcDRAM bidir only
	double capReadCellAccess; /* Capacitance of access device, Unit: ohm */ //gcDRAM bidir only
	double resWriteCellAccess; /* Resistance of access device, Unit: ohm */ //gcDRAM bidir only
	double capWriteCellAccess; /* Capacitance of access device, Unit: ohm */ //gcDRAM bidir only
	int largerLine;


	// 1.4 update : parameters for buffer insertion
	double widthInvN, widthInvP;
	double wInv, hInv, drivecapin, drivecapout, targetdriveres;
	double sectionres, sectioncap, sectionresMux, sectioncapMux;
	double activityRowRead, activityRowWrite;		// Activity for # of rows
	double gateCapRep;
	int numRepeaters;
	double bufferSizeRatio;

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
	Precharger	writecharger;
	SenseAmp	senseAmp;

	/* Monolithic 3D Update: Add MIV Grid for M3D connectivit*/
	TSV tsvArray;
};

#endif /* SUBARRAY_H_ */
