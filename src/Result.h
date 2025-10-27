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


#ifndef RESULT_H_
#define RESULT_H_

#include "BankWithHtree.h"
#include "BankWithoutHtree.h"
#include "Wire.h"

class Result {
public:
	Result();
	virtual ~Result();

	/* Functions */
	void print(int indent = 0);
	void printToFile(int indent = 0, const string &FileName = "out.txt");
	void printAsCache(Result &tagBank, CacheAccessMode cacheAccessMode);
	void printAsCacheToFile(CacheAccessMode cacheAccessMode, const string &FileName);
	void reset();
    bool nearReference(double reference, double measured);
	void printToCsvFile(ofstream &outputFile);
	void printAsCacheToCsvFile(Result &tagBank, CacheAccessMode cacheAccessMode, ofstream &outputFile);
	bool compareAndUpdate(Result &newResult);
    string printOptimizationTarget();

	OptimizationTarget optimizationTarget;	/* Exploration should not be assigned here */

	Bank * bank;
	Wire * localWire;		/* TO-DO: this one has the same name as one of the global variables */
	Wire * globalWire;

	double limitReadLatency;			/* The maximum allowable read latency, Unit: s */
	double limitWriteLatency;			/* The maximum allowable write latency, Unit: s */
	double limitReadDynamicEnergy;		/* The maximum allowable read dynamic energy, Unit: J */
	double limitWriteDynamicEnergy;		/* The maximum allowable write dynamic energy, Unit: J */
	double limitReadEdp;				/* The maximum allowable read EDP, Unit: s-J */
	double limitWriteEdp;				/* The maximum allowable write EDP, Unit: s-J */
	double limitArea;					/* The maximum allowable area, Unit: m^2 */
	double limitLeakage;				/* The maximum allowable leakage power, Unit: W */
    MemCell *cellTech;
};

#endif /* RESULT_H_ */
