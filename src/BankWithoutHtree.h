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


#ifndef BANKWITHOUTHTREE_H_
#define BANKWITHOUTHTREE_H_

#include "Bank.h"
#include "SubArray.h"
#include "typedef.h"
#include "Comparator.h"

class BankWithoutHtree: public Bank {
public:
	BankWithoutHtree();
	virtual ~BankWithoutHtree();

	/* Functions */
	void Initialize(int _numRowSubArray, int _numColumnSubArray, long long _capacity,
			long _blockSize, int _associativity, int _numRowPerSet, int _numActiveSubArrayPerRow,
			int _numActiveSubArrayPerColumn, int _muxSenseAmp, bool _internalSenseAmp, int _muxOutputLev1, int _muxOutputLev2,
			int _numRowMat, int _numColumnMat,
			int _numActiveMatPerRow, int _numActiveMatPerColumn,
			BufferDesignTarget _areaOptimizationLevel, MemoryType _memoryType,
            int _stackedDieCount, int _partitionGranularity, int monolithicStackCount);
	void CalculateArea();
	void CalculateRC();
	void CalculateLatencyAndPower();
	BankWithoutHtree & operator=(const BankWithoutHtree &);

	int numAddressBit;		   /* Number of bank address bits */
	int numWay;                  /* Number of way in a subarray */
	int numAddressBitRouteToSubArray;  /* Number of address bits routed to subarray */
	int numDataBitRouteToSubArray;   /* Number of data bits routed to subarray */

	Mux	globalBitlineMux;
	SenseAmp globalSenseAmp;
	Comparator globalComparator;
};

#endif /* BANKWITHOUTHTREE_H_ */
