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


#include "Result.h"
#include "global.h"
#include "formula.h"
#include "macros.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

namespace {

bool IsRefreshMemory(MemCellType type) {
	return type == DRAM || type == eDRAM || type == gcDRAM;
}

const char *M3DDominantTierName(M3DDominantTier tier) {
	switch (tier) {
	case M3DDominantTier::logic:
		return "logic";
	case M3DDominantTier::memory:
		return "memory";
	default:
		return "none";
	}
}

void PrintOperatingPoint(ostream &output, int indent, const char *name,
		const AOSDeviceOperatingPoint &point) {
	ios::fmtflags flags = output.flags();
	streamsize precision = output.precision();
	output << scientific << setprecision(6);
	output << string(indent, ' ') << " - " << name << " AOS operating point: Ion="
			<< point.Ion << "A, Ioff=" << point.Ioff << "A, Ron=" << point.Ron
			<< "ohm, Roff=" << point.Roff << "ohm, Cgate=" << point.Cgate
			<< "F, Cdrain=" << point.Cdrain << "F" << endl;
	output.flags(flags);
	output.precision(precision);
}

void PrintIntegratedModelLabels(ostream &output, int indent, const Mat &mat,
		const MemCell &memoryCell) {
	if (memoryCell.memCellType == DRAM || memoryCell.memCellType == eDRAM)
		output << string(indent, ' ') << "DRAM timing model: restore-aware v1" << endl;
	if (memoryCell.memCellType == gcDRAM)
		output << string(indent, ' ') << "gcDRAM electrical/energy model: split-path v1" << endl;
	if (memoryCell.oxideTransistor) {
		output << string(indent, ' ') << "AOS compact-model v1" << endl;
		if (memoryCell.memCellType == eDRAM)
			PrintOperatingPoint(output, indent, "Access", mat.aosAccessOperatingPoint);
		else if (memoryCell.memCellType == gcDRAM) {
			PrintOperatingPoint(output, indent, "Read", mat.aosReadOperatingPoint);
			PrintOperatingPoint(output, indent, "Write", mat.aosWriteOperatingPoint);
		}
	}
}

void PrintM3DDetails(ostream &output, int indent, const Mat &mat) {
	output << string(indent, ' ') << " |--- M3D footprint/MIV model = corrected v1" << endl;
	output << string(indent, ' ') << " |--- Mat Memory Tiers = " << mat.stackedMemTiers << endl;
	output << string(indent, ' ') << " |--- MIVs Per Tier = " << mat.m3d.mivsPerTier << endl;
	output << string(indent, ' ') << " |--- Total MIV Count = " << mat.m3d.totalMivCount << endl;
	output << string(indent, ' ') << " |--- Total MIV Area = " << TO_SQM(mat.m3d.totalMivArea) << endl;
	output << string(indent, ' ') << " |--- Base Peripheral Logic Area = "
			<< TO_SQM(mat.m3d.peripheralLogicArea) << endl;
	output << string(indent, ' ') << " |--- Final Logic-Layer Area = "
			<< TO_SQM(mat.m3d.finalLogicLayerArea) << endl;
	output << string(indent, ' ') << " |--- Per-Tier Memory Area = "
			<< TO_SQM(mat.m3d.perTierMemoryArea) << endl;
	output << string(indent, ' ') << " |--- Projected MAT Area = "
			<< TO_SQM(mat.m3d.projectedArea) << endl;
	output << string(indent, ' ') << " |--- Dominant Tier = "
			<< M3DDominantTierName(mat.m3d.dominantTier) << endl;
}

void PrintIntegratedTimingDetails(ostream &output, int indent, const Mat &mat,
		MemCellType type) {
	if (type == DRAM || type == eDRAM) {
		output << string(indent, ' ') << "       |--- DRAM Access (Time-to-Data) = "
				<< TO_SECOND(mat.dramTiming.accessLatency) << endl;
		output << string(indent, ' ') << "       |--- DRAM Restore Delay = "
				<< TO_SECOND(mat.dramTiming.restoreDelay) << endl;
		output << string(indent, ' ') << "       |--- DRAM Full Read Cycle = "
				<< TO_SECOND(mat.dramTiming.readCycleLatency) << endl;
		output << string(indent, ' ') << "       |--- DRAM Write-Bitline Settling = "
				<< TO_SECOND(mat.dramTiming.writeBitlineDelay) << endl;
	} else if (type == gcDRAM) {
		output << string(indent, ' ') << "       |--- gcDRAM Read-Bitline Delay = "
				<< TO_SECOND(mat.readBitlineDelay) << endl;
		output << string(indent, ' ') << "       |--- gcDRAM Write-Bitline Delay = "
				<< TO_SECOND(mat.writeBitlineDelay) << endl;
	}
}

void PrintGcDRAMReadEnergy(ostream &output, int indent, const Mat &mat,
		MemCellType type) {
	if (type == gcDRAM)
		output << string(indent, ' ') << "       |--- gcDRAM Read-Bitline/Access Energy = "
				<< TO_JOULE(mat.gcDramPower.readBitlineAccessEnergy) << endl;
}

void PrintGcDRAMWriteEnergy(ostream &output, int indent, const Mat &mat,
		MemCellType type) {
	if (type == gcDRAM) {
		output << string(indent, ' ') << "       |--- gcDRAM Write-Bitline/Access Energy = "
				<< TO_JOULE(mat.gcDramPower.writeBitlineAccessEnergy) << endl;
		output << string(indent, ' ') << "       |--- gcDRAM Write-Charge-Driver Energy = "
				<< TO_JOULE(mat.gcDramPower.writeChargeDriverEnergy) << endl;
	}
}

void PrintGcDRAMRefreshEnergy(ostream &output, int indent, const Mat &mat,
		MemCellType type) {
	if (type != gcDRAM)
		return;

	const double perRowEnergy = mat.gcDramPower.readBitlineAccessEnergy
			+ mat.gcDramPower.writeBitlineAccessEnergy
			+ mat.gcRowDecoder.readDynamicEnergy + mat.rowDecoder.readDynamicEnergy
			+ mat.precharger.readDynamicEnergy + mat.gcDramPower.writeChargeDriverEnergy
			+ mat.senseAmp.readDynamicEnergy;
	const long long rowMultiplier = mat.numRow + 2;

	/* gcDRAM refreshDynamicEnergy is an aggregate MAT sweep.  Expose every
	 * per-row term and the retained (numRow + 2) multiplier so the printed
	 * decomposition can be reconstructed without mixing units/scopes. */
	output << string(indent, ' ') << "       |--- gcDRAM Refresh Read-Bitline/Access Energy Per Row = "
			<< TO_JOULE(mat.gcDramPower.readBitlineAccessEnergy) << endl;
	output << string(indent, ' ') << "       |--- gcDRAM Refresh Write-Bitline/Access Energy Per Row = "
			<< TO_JOULE(mat.gcDramPower.writeBitlineAccessEnergy) << endl;
	output << string(indent, ' ') << "       |--- gcDRAM Refresh Read-Row-Decoder Energy Per Row = "
			<< TO_JOULE(mat.gcRowDecoder.readDynamicEnergy) << endl;
	output << string(indent, ' ') << "       |--- gcDRAM Refresh Write-Row-Decoder Energy Per Row = "
			<< TO_JOULE(mat.rowDecoder.readDynamicEnergy) << endl;
	output << string(indent, ' ') << "       |--- gcDRAM Refresh Read-Precharger Energy Per Row = "
			<< TO_JOULE(mat.precharger.readDynamicEnergy) << endl;
	output << string(indent, ' ') << "       |--- gcDRAM Refresh Write-Charger Energy Per Row = "
			<< TO_JOULE(mat.gcDramPower.writeChargeDriverEnergy) << endl;
	output << string(indent, ' ') << "       |--- gcDRAM Refresh Sense-Amp Energy Per Row = "
			<< TO_JOULE(mat.senseAmp.readDynamicEnergy) << endl;
	output << string(indent, ' ') << "       |--- gcDRAM Refresh Energy Per-Row Sum = "
			<< TO_JOULE(perRowEnergy) << endl;
	output << string(indent, ' ') << "       |--- gcDRAM Refresh Row Multiplier = "
			<< rowMultiplier << endl;
	output << string(indent, ' ') << "       |--- gcDRAM Reconstructed Mat Refresh Energy = "
			<< TO_JOULE(perRowEnergy * rowMultiplier) << endl;
}

void PrintAOSLeakageUpperBound(ostream &output, int indent, const Mat &mat) {
	if (mat.aosLeakageUpperBound > 0) {
		output << string(indent, ' ') << " -- AOS full-Vds leakage upper bound = "
				<< TO_WATT(mat.aosLeakageUpperBound)
				<< " (uncalibrated conservative bound)" << endl;
	}
}

} // namespace

Result::Result() {
	// TODO Auto-generated constructor stub
	if (inputParameter->routingMode == h_tree)
		bank = new BankWithHtree();
	else
		bank = new BankWithoutHtree();
	localWire = new Wire();
	globalWire = new Wire();

	/* initialize the worst case */
	bank->readLatency = invalid_value;
	bank->writeLatency = invalid_value;
	bank->readDynamicEnergy = invalid_value;
	bank->writeDynamicEnergy = invalid_value;
	bank->leakage = invalid_value;
	bank->height = invalid_value;
	bank->width = invalid_value;
	bank->area = invalid_value;

	/* No constraints */
	limitReadLatency = invalid_value;
	limitWriteLatency = invalid_value;
	limitReadDynamicEnergy = invalid_value;
	limitWriteDynamicEnergy = invalid_value;
	limitReadEdp = invalid_value;
	limitWriteEdp = invalid_value;
	limitReadBandwidth = invalid_value_min;
	limitWriteBandwidth = invalid_value_min;
	limitArea = invalid_value;
	limitLeakage = invalid_value;

	/* Default read latency optimization */
	optimizationTarget = read_latency_optimized;

	/* Capture the explored cell so saved results do not depend on the mutable global. */
	cellTech = cell;
}

Result::~Result() {
	// TODO Auto-generated destructor stub
	if (bank)
		delete bank;
	if (Result::localWire)
		delete Result::localWire;
	if (Result::globalWire)
		delete Result::globalWire;
}

void Result::reset() {
	bank->readLatency = invalid_value;
	bank->writeLatency = invalid_value;
	bank->readDynamicEnergy = invalid_value;
	bank->writeDynamicEnergy = invalid_value;
	bank->leakage = invalid_value;
	bank->height = invalid_value;
	bank->width = invalid_value;
	bank->area = invalid_value;
}

double Result::getReadBandwidth() const {
	if (!cellTech || !std::isfinite(bank->readLatency)
			|| bank->readLatency >= invalid_value / 10 || bank->blockSize <= 0)
		return 0;

	double readCycleLatency = bank->subarray.mat.readLatency - bank->subarray.mat.rowDecoder.readLatency
			+ bank->subarray.mat.precharger.readLatency;
	if (cellTech->memCellType == DRAM || cellTech->memCellType == eDRAM) {
		readCycleLatency = bank->subarray.mat.dramTiming.readCycleLatency;
	} else if (cellTech->memCellType == gcDRAM) {
		readCycleLatency = bank->subarray.mat.readLatency - bank->subarray.mat.gcRowDecoder.readLatency
				+ bank->subarray.mat.precharger.readLatency;
	}
	if (!std::isfinite(readCycleLatency)
			|| readCycleLatency <= 0 || readCycleLatency >= invalid_value / 10)
		return 0;

	return (double)bank->blockSize / readCycleLatency / 8;
}

double Result::getWriteBandwidth() const {
	if (!cellTech || !std::isfinite(bank->writeLatency)
			|| bank->writeLatency >= invalid_value / 10 || bank->blockSize <= 0)
		return 0;

	double writeCycleLatency = bank->subarray.mat.writeLatency;
	if (!std::isfinite(writeCycleLatency)
			|| writeCycleLatency <= 0 || writeCycleLatency >= invalid_value / 10)
		return 0;

	return (double)bank->blockSize / writeCycleLatency / 8;
}

bool Result::compareAndUpdate(Result &newResult) {
    bool toUpdate = false;

	if (newResult.bank->readLatency <= limitReadLatency && newResult.bank->writeLatency <= limitWriteLatency
			&& newResult.bank->readDynamicEnergy <= limitReadDynamicEnergy && newResult.bank->writeDynamicEnergy <= limitWriteDynamicEnergy
			&& newResult.bank->readLatency * newResult.bank->readDynamicEnergy <= limitReadEdp
			&& newResult.bank->writeLatency * newResult.bank->writeDynamicEnergy <= limitWriteEdp
			&& newResult.getReadBandwidth() >= limitReadBandwidth
			&& newResult.getWriteBandwidth() >= limitWriteBandwidth
			&& newResult.bank->area <= limitArea && newResult.bank->leakage <= limitLeakage) {
		switch (optimizationTarget) {
		case read_latency_optimized:
			if 	(newResult.bank->readLatency < bank->readLatency)
				toUpdate = true;
			break;
		case write_latency_optimized:
			if 	(newResult.bank->writeLatency < bank->writeLatency)
				toUpdate = true;
			break;
		case read_energy_optimized:
			if 	(newResult.bank->readDynamicEnergy < bank->readDynamicEnergy)
				toUpdate = true;
			break;
		case write_energy_optimized:
			if 	(newResult.bank->writeDynamicEnergy < bank->writeDynamicEnergy)
				toUpdate = true;
			break;
		case read_edp_optimized:
			if 	(newResult.bank->readLatency * newResult.bank->readDynamicEnergy < bank->readLatency * bank->readDynamicEnergy)
				toUpdate = true;
			break;
		case write_edp_optimized:
			if 	(newResult.bank->writeLatency * newResult.bank->writeDynamicEnergy < bank->writeLatency * bank->writeDynamicEnergy)
				toUpdate = true;
			break;
		case read_bandwidth_optimized:
			if 	(newResult.getReadBandwidth() > getReadBandwidth())
				toUpdate = true;
			break;
		case write_bandwidth_optimized:
			if 	(newResult.getWriteBandwidth() > getWriteBandwidth())
				toUpdate = true;
			break;
		case area_optimized:
			if 	(newResult.bank->area < bank->area)
				toUpdate = true;
			break;
		case leakage_optimized:
			if 	(newResult.bank->leakage < bank->leakage)
				toUpdate = true;
			break;
		default:	/* Exploration */
			/* should not happen */
			;
		}
		if (toUpdate) {
			*bank = *(newResult.bank);
			*localWire = *(newResult.localWire);
			*globalWire = *(newResult.globalWire);
			cellTech = newResult.cellTech;
		}
	}

    return toUpdate;
}

string Result::printOptimizationTarget() {
    string rv;

    switch (optimizationTarget) {
    case read_latency_optimized:
        rv = "Read Latency";
        break;
    case write_latency_optimized:
        rv = "Write Latency";
        break;
    case read_energy_optimized:
        rv = "Read Energy";
        break;
    case write_energy_optimized:
        rv = "Write Energy";
        break;
    case read_edp_optimized:
        rv = "Read Energy-Delay-Product";
        break;
    case write_edp_optimized:
        rv = "Write Energy-Delay-Product";
        break;
    case read_bandwidth_optimized:
        rv = "Read Bandwidth";
        break;
    case write_bandwidth_optimized:
        rv = "Write Bandwidth";
        break;
    case area_optimized:
        rv = "Area";
        break;
    case leakage_optimized:
        rv = "Leakage";
        break;
    default:	/* Exploration */
        /* should not happen */
        ;
    }

    return rv;
}

void Result::print(int indent) {
	/* Deliberately shadow the process-wide exploration pointer with this snapshot's cell. */
	MemCell *const cell = cellTech;
	if (!cell) {
		cerr << "Result has no associated memory cell." << endl;
		return;
	}
	cout << string(indent, ' ') << endl;
    cout << string(indent, ' ') << "=============" << endl;
    cout << string(indent, ' ') << "   SUMMARY   " << endl;
    cout << string(indent, ' ') << "=============" << endl;
    cout << string(indent, ' ') << "Optimized for: " << printOptimizationTarget() << endl;
    cellTech->PrintCell(indent);
	cout << string(indent, ' ') << endl;
    cout << string(indent, ' ') << "=============" << endl;
    cout << string(indent, ' ') << "CONFIGURATION" << endl;
    cout << string(indent, ' ') << "=============" << endl;
    if (bank->stackedDieCount > 1) {
        cout << string(indent, ' ') << "Bank Organization: " << bank->numRowSubArray << " x " << bank->numColumnSubArray << " x " << bank->stackedDieCount << endl;
        cout << string(indent, ' ') << " - Row Activation   : " << bank->numActiveSubArrayPerColumn << " / " << bank->numRowSubArray << " x 1" << endl;
        cout << string(indent, ' ') << " - Column Activation: " << bank->numActiveSubArrayPerRow << " / " << bank->numColumnSubArray << " x 1" << endl;
    } else {
        cout << string(indent, ' ') << "Bank Organization: " << bank->numRowSubArray << " x " << bank->numColumnSubArray << endl;
        cout << string(indent, ' ') << " - Row Activation   : " << bank->numActiveSubArrayPerColumn << " / " << bank->numRowSubArray << endl;
        cout << string(indent, ' ') << " - Column Activation: " << bank->numActiveSubArrayPerRow << " / " << bank->numColumnSubArray << endl;
    }
	cout << string(indent, ' ') << "SubArray Organization: " << bank->numRowMat << " x " << bank->numColumnMat << endl;
	cout << string(indent, ' ') << " - Row Activation   : " << bank->numActiveMatPerColumn << " / " << bank->numRowMat << endl;
	cout << string(indent, ' ') << " - Column Activation: " << bank->numActiveMatPerRow << " / " << bank->numColumnMat << endl;
	cout << string(indent, ' ') << " - Mat Size    : " << bank->subarray.mat.numRow << " Rows x " << bank->subarray.mat.numColumn << " Columns" << endl;
	PrintIntegratedModelLabels(cout, indent, bank->subarray.mat, *cell);
	cout << string(indent, ' ') << "Mux Level:" << endl;
	cout << string(indent, ' ') << " - Senseamp Mux      : " << bank->muxSenseAmp << endl;
	cout << string(indent, ' ') << " - Output Level-1 Mux: " << bank->muxOutputLev1 << endl;
	cout << string(indent, ' ') << " - Output Level-2 Mux: " << bank->muxOutputLev2 << endl;
	if (inputParameter->designTarget == cache)
		cout << string(indent, ' ') << " - One set is partitioned into " << bank->numRowPerSet << " rows" << endl;
	cout << string(indent, ' ') << "Local Wire:" << endl;
	cout << string(indent, ' ') << " - Wire Type : ";
	switch (localWire->wireType) {
	case local_aggressive:
		cout << string(indent, ' ') << "Local Aggressive" << endl;
		break;
	case local_conservative:
		cout << string(indent, ' ') << "Local Conservative" << endl;
		break;
	case semi_aggressive:
		cout << string(indent, ' ') << "Semi-Global Aggressive" << endl;
		break;
	case semi_conservative:
		cout << string(indent, ' ') << "Semi-Global Conservative" << endl;
		break;
	case global_aggressive:
		cout << string(indent, ' ') << "Global Aggressive" << endl;
		break;
	case global_conservative:
		cout << string(indent, ' ') << "Global Conservative" << endl;
		break;
	default:
		cout << string(indent, ' ') << "DRAM Wire" << endl;
	}
	cout << string(indent, ' ') << " - Repeater Type: ";
	switch (localWire->wireRepeaterType) {
	case repeated_none:
		cout << string(indent, ' ') << "No Repeaters" << endl;
		break;
	case repeated_opt:
		cout << string(indent, ' ') << "Fully-Optimized Repeaters" << endl;
		break;
	case repeated_5:
		cout << string(indent, ' ') << "Repeaters with 5% Overhead" << endl;
		break;
	case repeated_10:
		cout << string(indent, ' ') << "Repeaters with 10% Overhead" << endl;
		break;
	case repeated_20:
		cout << string(indent, ' ') << "Repeaters with 20% Overhead" << endl;
		break;
	case repeated_30:
		cout << string(indent, ' ') << "Repeaters with 30% Overhead" << endl;
		break;
	case repeated_40:
		cout << string(indent, ' ') << "Repeaters with 40% Overhead" << endl;
		break;
	case repeated_50:
		cout << string(indent, ' ') << "Repeaters with 50% Overhead" << endl;
		break;
	default:
		cout << string(indent, ' ') << "Unknown" << endl;
	}
	cout << string(indent, ' ') << " - Low Swing : ";
	if (localWire->isLowSwing)
		cout << string(indent, ' ') << "Yes" << endl;
	else
		cout << string(indent, ' ') << "No" << endl;
	cout << string(indent, ' ') << "Global Wire:" << endl;
	cout << string(indent, ' ') << " - Wire Type : ";
	switch (globalWire->wireType) {
	case local_aggressive:
		cout << string(indent, ' ') << "Local Aggressive" << endl;
		break;
	case local_conservative:
		cout << string(indent, ' ') << "Local Conservative" << endl;
		break;
	case semi_aggressive:
		cout << string(indent, ' ') << "Semi-Global Aggressive" << endl;
		break;
	case semi_conservative:
		cout << string(indent, ' ') << "Semi-Global Conservative" << endl;
		break;
	case global_aggressive:
		cout << string(indent, ' ') << "Global Aggressive" << endl;
		break;
	case global_conservative:
		cout << string(indent, ' ') << "Global Conservative" << endl;
		break;
	default:
		cout << string(indent, ' ') << "DRAM Wire" << endl;
	}
	cout << string(indent, ' ') << " - Repeater Type: ";
	switch (globalWire->wireRepeaterType) {
	case repeated_none:
		cout << string(indent, ' ') << "No Repeaters" << endl;
		break;
	case repeated_opt:
		cout << string(indent, ' ') << "Fully-Optimized Repeaters" << endl;
		break;
	case repeated_5:
		cout << string(indent, ' ') << "Repeaters with 5% Overhead" << endl;
		break;
	case repeated_10:
		cout << string(indent, ' ') << "Repeaters with 10% Overhead" << endl;
		break;
	case repeated_20:
		cout << string(indent, ' ') << "Repeaters with 20% Overhead" << endl;
		break;
	case repeated_30:
		cout << string(indent, ' ') << "Repeaters with 30% Overhead" << endl;
		break;
	case repeated_40:
		cout << string(indent, ' ') << "Repeaters with 40% Overhead" << endl;
		break;
	case repeated_50:
		cout << string(indent, ' ') << "Repeaters with 50% Overhead" << endl;
		break;
	default:
		cout << string(indent, ' ') << "Unknown" << endl;
	}
	cout << string(indent, ' ') << " - Low Swing : ";
	if (globalWire->isLowSwing)
		cout << string(indent, ' ') << "Yes" << endl;
	else
		cout << string(indent, ' ') << "No" << endl;
	cout << string(indent, ' ') << "Buffer Design Style: ";
	switch (bank->areaOptimizationLevel) {
	case latency_first:
		cout << string(indent, ' ') << "Latency-Optimized" << endl;
		break;
	case area_first:
		cout << string(indent, ' ') << "Area-Optimized" << endl;
		break;
	default:	/* balance */
		cout << string(indent, ' ') << "Balanced" << endl;
	}

	cout << string(indent, ' ') << "=============" << endl;
    cout << string(indent, ' ') << "   RESULT" << endl;
    cout << string(indent, ' ') << "=============" << endl;

	cout << string(indent, ' ') << "Area:" << endl;

	cout << string(indent, ' ') << " - Total Area = " << TO_METER(bank->height) << " x " << TO_METER(bank->width)
			<< " = " << TO_SQM(bank->area) << endl;
	cout << string(indent, ' ') << " |--- SubArray Area      = " << TO_METER(bank->subarray.height) << " x " << TO_METER(bank->subarray.width)
			<< " = " << TO_SQM(bank->subarray.area) << "   (" << cell->area * tech->featureSize * tech->featureSize
			* bank->capacity / bank->numRowSubArray / bank->numColumnSubArray / bank->subarray.area * 100 << "%)" << endl;
	cout << string(indent, ' ') << " |--- Mat Area = " << TO_METER(bank->subarray.mat.height) << " x "
			<< TO_METER(bank->subarray.mat.width) << " = " << TO_SQM(bank->subarray.mat.area) << "   ("
			<< cell->area * tech->featureSize * tech->featureSize * bank->capacity / bank->numRowSubArray
			/ bank->numColumnSubArray / bank->numRowMat / bank->numColumnMat
			/ bank->subarray.mat.area * 100 << "%)" <<endl;
	
	/* Mat Area BreakDown */
	if (inputParameter->viewMatStats){
		cout << string(indent, ' ') << " |--- Mat rowDecoder Area = " << TO_SQM(2*bank->subarray.mat.rowDecoder.area) << endl;
		if (cell->memCellType == gcDRAM)
			cout << string(indent, ' ') << " |--- Mat gcDRAM Read Row Decoder Area = " << TO_SQM(bank->subarray.mat.gcRowDecoder.area) << endl;
		cout << string(indent, ' ') << " |--- Mat bitlineMuxDecoder Area = " << TO_SQM(2*bank->subarray.mat.bitlineMuxDecoder.area) << endl;
		cout << string(indent, ' ') << " |--- Mat bitlineMux Area = " << TO_SQM(bank->subarray.mat.bitlineMux.area) << endl;
		cout << string(indent, ' ') << " |--- Mat senseAmpMuxLev1 Area = " << TO_SQM(bank->subarray.mat.senseAmpMuxLev1.area) << endl;
		cout << string(indent, ' ') << " |--- Mat senseAmpMuxLev2 Area = " << TO_SQM(bank->subarray.mat.senseAmpMuxLev2.area) << endl;
		cout << string(indent, ' ') << " |--- Mat senseAmpMuxLev1Decoder Area = " << TO_SQM(bank->subarray.mat.senseAmpMuxLev1Decoder.area) << endl;
		cout << string(indent, ' ') << " |--- Mat senseAmpMuxLev2Decoder Area = " << TO_SQM(bank->subarray.mat.senseAmpMuxLev2Decoder.area) << endl;
		cout << string(indent, ' ') << " |--- Mat precharger Area = " << TO_SQM(2*bank->subarray.mat.precharger.area) << endl;
		if (cell->memCellType == gcDRAM)
			cout << string(indent, ' ') << " |--- Mat gcDRAM Write Charger Area = " << TO_SQM(bank->subarray.mat.writecharger.area) << endl;
		cout << string(indent, ' ') << " |--- Mat senseAmp Area = " << TO_SQM(bank->subarray.mat.senseAmp.area) << endl;
		cout << string(indent, ' ') << " |--- Mat MIV Area = " << TO_SQM(bank->subarray.mat.tsvArray.area) << endl;
	}

	/*Mat Area BreakDown End*/
	
	if (inputParameter->monolithic3DMat){
		PrintM3DDetails(cout, indent, bank->subarray.mat);
	}
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        cout << string(indent, ' ') << " |--- TSV Area      = " << TO_SQM(bank->tsvArray.area) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        double totalTSVArea = bank->tsvArray.area + bank->subarray.tsvArray.area
                              * bank->numColumnSubArray * bank->numRowSubArray;
        double areaLogicLayer = bank->subarray.areaAllLogicBlocks * bank->numColumnSubArray
                              * bank->numRowSubArray;

        cout << string(indent, ' ') << " |--- TSV Area      = " << TO_SQM(totalTSVArea) << endl;
        cout << string(indent, ' ') << " |--- Logic Layer Area = " << TO_SQM(areaLogicLayer) << endl;
    }
	cout << string(indent, ' ') << " - Area Efficiency = " << cell->area * tech->featureSize * tech->featureSize
			* bank->capacity / bank->area * 100 << "%" << endl;

	cout << string(indent, ' ') << "Timing:" << endl;

	cout << string(indent, ' ') << " -  Read Latency = " << TO_SECOND(bank->readLatency) << endl;
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        double totalTSVLatency = (bank->tsvArray.readLatency + bank->tsvArray.writeLatency) * (bank->stackedDieCount-1);
        cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(totalTSVLatency) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        double totalTSVLatency = bank->tsvArray.readLatency * (bank->stackedDieCount-1)
                               + bank->subarray.tsvArray.writeLatency * (bank->stackedDieCount-1);

        cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(totalTSVLatency) << endl;
    }
	if (inputParameter->routingMode == h_tree)
		cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingReadLatency) << endl;
	else
		cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingReadLatency) << endl;
	cout << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.readLatency) << endl;
	cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
	cout << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.readLatency) << endl;
	PrintIntegratedTimingDetails(cout, indent, bank->subarray.mat, cell->memCellType);
	if(cell->memCellType != gcDRAM) { 
		cout << string(indent, ' ') << "       |--- Row Decoder Latency = " << TO_SECOND(bank->subarray.mat.rowDecoder.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Bitline Latency     = " << TO_SECOND(bank->subarray.mat.bitlineDelay) << endl;
		if (inputParameter->internalSensing)
			cout << string(indent, ' ') << "       |--- Senseamp Latency    = " << TO_SECOND(bank->subarray.mat.senseAmp.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Precharge Latency   = " << TO_SECOND(bank->subarray.mat.precharger.readLatency) << endl;
	}
	else {
		cout << string(indent, ' ') << "       |--- Read Row Decoder Latency = " << TO_SECOND(bank->subarray.mat.gcRowDecoder.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Write Row Decoder Latency = " << TO_SECOND(bank->subarray.mat.rowDecoder.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Bitline Latency     = " << TO_SECOND(bank->subarray.mat.readBitlineDelay) << endl;
		if (inputParameter->internalSensing)
			cout << string(indent, ' ') << "       |--- Senseamp Latency    = " << TO_SECOND(bank->subarray.mat.senseAmp.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Precharge Latency   = " << TO_SECOND(bank->subarray.mat.precharger.readLatency) << endl;
		cout << string(indent, ' ') << "       |--- Write Drive Latency   = " << TO_SECOND(bank->subarray.mat.writecharger.readLatency) << endl;
	}

	cout << string(indent, ' ') << "       |--- Mux Latency         = " << TO_SECOND(bank->subarray.mat.bitlineMux.readLatency
													+ bank->subarray.mat.senseAmpMuxLev1.readLatency
													+ bank->subarray.mat.senseAmpMuxLev2.readLatency) << endl;
	if (bank->subarray.memoryType == MemoryType::tag && bank->subarray.internalSenseAmp)
		cout << string(indent, ' ') << "    |--- Comparator Latency  = " << TO_SECOND(bank->subarray.comparator.readLatency) << endl;

	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM ||
			(cell->memCellType == memristor && (cell->accessType == CMOS_access || cell->accessType == BJT_access))) {
		cout << string(indent, ' ') << " - RESET Latency = " << TO_SECOND(bank->resetLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		cout << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.resetLatency) << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
		cout << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.resetLatency) << endl;
		cout << string(indent, ' ') << "       |--- RESET Pulse Duration = " << TO_SECOND(cell->resetPulse) << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Latency  = " << TO_SECOND(bank->subarray.mat.rowDecoder.writeLatency) << endl;
		cout << string(indent, ' ') << "       |--- Charge Latency   = " << TO_SECOND(bank->subarray.mat.chargeLatency) << endl;
		cout << string(indent, ' ') << " - SET Latency   = " << TO_SECOND(bank->setLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.setLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.setLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		cout << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.setLatency) << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
		cout << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.setLatency) << endl;
		cout << string(indent, ' ') << "       |--- SET Pulse Duration   = " << TO_SECOND(cell->setPulse) << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Latency  = " << TO_SECOND(bank->subarray.mat.rowDecoder.writeLatency) << endl;
		cout << string(indent, ' ') << "       |--- Charger Latency      = " << TO_SECOND(bank->subarray.mat.chargeLatency) << endl;
	} else if (cell->memCellType == SLCNAND) {
		cout << string(indent, ' ') << " - Erase Latency = " << TO_SECOND(bank->resetLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		cout << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.resetLatency) << endl;
		cout << string(indent, ' ') << " - Programming Latency   = " << TO_SECOND(bank->setLatency) << endl;
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		cout << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.setLatency) << endl;
	} else {
		cout << string(indent, ' ') << " - Write Latency = " << TO_SECOND(bank->writeLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingWriteLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingWriteLatency) << endl;
		cout << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.writeLatency) << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
		cout << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.writeLatency) << endl;
		if (cell->memCellType == MRAM)
			cout << string(indent, ' ') << "       |--- Write Pulse Duration = " << TO_SECOND(cell->resetPulse) << endl;	// MRAM reset/set is equal
		cout << string(indent, ' ') << "       |--- Row Decoder Latency = " << TO_SECOND(bank->subarray.mat.rowDecoder.writeLatency) << endl;
		cout << string(indent, ' ') << "       |--- Charge Latency      = " << TO_SECOND(bank->subarray.mat.chargeLatency) << endl;
		if (cell->memCellType == gcDRAM) cout << string(indent, ' ') << "       |--- Write Bitline Latency     = " << TO_SECOND(bank->subarray.mat.writeBitlineDelay) << endl;
		}
	    if (IsRefreshMemory(cell->memCellType)) {
        cout << string(indent, ' ') << " - Refresh Latency = " << TO_SECOND(bank->refreshLatency) << endl;
        if ((bank->stackedDieCount > 1 && bank->partitionGranularity == 0)) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } if (bank->subarray.mat.stackedMemTiers) {
			//cout << string(indent, ' ') << " |--- MIV Latency    = " << TO_SECOND(bank->subarray.mat.tsvArray.writeLatency * (bank->subarray.mat.stackedMemTiers)) << endl;
			//cout << string(indent, ' ') << " |--- BL Res    = " << TO_SECOND(bank->subarray.mat.resBitline) << endl;
			//cout << string(indent, ' ') << " |--- BL Cap    = " << TO_SECOND(bank->subarray.mat.capBitline) << endl;
		}
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingRefreshLatency) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingRefreshLatency) << endl;
		cout << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.refreshLatency) << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
		cout << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.refreshLatency) << endl;
    }

	double readBandwidth = getReadBandwidth();
	//cout << "DEBUGGING: bank blockSize - " << bank->blockSize << endl;
	//cout << "DEBUGGING: mat readLatency - " << TO_SECOND(bank->subarray.mat.readLatency) << ", decoder readLatency - " << TO_SECOND(bank->subarray.mat.rowDecoder.readLatency) << ", precharger latency - " << TO_SECOND(bank->subarray.mat.precharger.readLatency) << endl;
	cout << string(indent, ' ') << " - Read Bandwidth  = " << TO_BPS(readBandwidth) << endl;
	double writeBandwidth = getWriteBandwidth();
	cout << string(indent, ' ') << " - Write Bandwidth = " << TO_BPS(writeBandwidth) << endl;

	cout << string(indent, ' ') << "Power:" << endl;

	cout << string(indent, ' ') << " -  Read Dynamic Energy = " << TO_JOULE(bank->readDynamicEnergy) << endl;
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        // address and control bit dynamics + read data output dynamic
        cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits + bank->tsvArray.readDynamicEnergy * bank->tsvArray.numDataBits * (bank->stackedDieCount-1)) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        // address bit dynamic + control bit dynamic + read data dynamic
        // TODO: revisit this
        cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->subarray.tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * (bank->subarray.tsvArray.numAccessBits) + bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->stackedDieCount + bank->tsvArray.readDynamicEnergy * bank->blockSize * (bank->stackedDieCount-1)) << endl;
    }
	if (inputParameter->routingMode == h_tree)
		cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingReadDynamicEnergy) << endl;
	else
		cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingReadDynamicEnergy) << endl;
	cout << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.readDynamicEnergy) << " per subarray" << endl;
	cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.readDynamicEnergy - bank->subarray.mat.readDynamicEnergy
														* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
														<< endl;
	cout << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.readDynamicEnergy) << " per active mat" << endl;\
	if (cell->memCellType != gcDRAM)
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.readDynamicEnergy) << endl;
	else
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.gcRowDecoder.readDynamicEnergy) << endl;
	cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.readDynamicEnergy
													+ bank->subarray.mat.senseAmpMuxLev1Decoder.readDynamicEnergy
													+ bank->subarray.mat.senseAmpMuxLev2Decoder.readDynamicEnergy) << endl;
	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM || cell->memCellType == MRAM || cell->memCellType == memristor ) {
		cout << string(indent, ' ') << "       |--- Bitline & Cell Read Energy = " << TO_JOULE(bank->subarray.mat.cellReadEnergy) << endl;
	}
	if (inputParameter->internalSensing)
		cout << string(indent, ' ') << "       |--- Senseamp Dynamic Energy    = " << TO_JOULE(bank->subarray.mat.senseAmp.readDynamicEnergy) << endl;
	cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.readDynamicEnergy
													+ bank->subarray.mat.senseAmpMuxLev1.readDynamicEnergy
													+ bank->subarray.mat.senseAmpMuxLev2.readDynamicEnergy) << endl;
	cout << string(indent, ' ') << "       |--- Precharge Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.precharger.readDynamicEnergy) << endl;
	PrintGcDRAMReadEnergy(cout, indent, bank->subarray.mat, cell->memCellType);

	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM ||
			(cell->memCellType == memristor && (cell->accessType == CMOS_access || cell->accessType == BJT_access))) {
		cout << string(indent, ' ') << " - RESET Dynamic Energy = " << TO_JOULE(bank->resetDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.resetDynamicEnergy) << " per subarray" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Cell RESET Dynamic Energy  = " << TO_JOULE(bank->subarray.mat.cellResetEnergy) << endl;
		cout << string(indent, ' ') << " - SET Dynamic Energy = " << TO_JOULE(bank->setDynamicEnergy) << endl;
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.setDynamicEnergy) << " per subarray" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Cell SET Dynamic Energy    = " << TO_JOULE(bank->subarray.mat.cellSetEnergy) << endl;
	} else if (cell->memCellType == SLCNAND) {
		cout << string(indent, ' ') << " - Erase Dynamic Energy = " << TO_JOULE(bank->resetDynamicEnergy) << " per block" << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.resetDynamicEnergy) << " per subarray" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << " - Programming Dynamic Energy = " << TO_JOULE(bank->setDynamicEnergy) << " per page" << endl;
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.setDynamicEnergy) << " per subarray" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
	} else {
		cout << string(indent, ' ') << " - Write Dynamic Energy = " << TO_JOULE(bank->writeDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingWriteDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingWriteDynamicEnergy) << endl;
		cout << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.writeDynamicEnergy) << " per subarray" << endl;
		cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		cout << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		cout << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		if (cell->memCellType == MRAM) {
			cout << string(indent, ' ') << "       |--- Bitline & Cell Write Energy= " << TO_JOULE(bank->subarray.mat.cellResetEnergy) << endl;
		}
	}
	PrintGcDRAMWriteEnergy(cout, indent, bank->subarray.mat, cell->memCellType);
    if (IsRefreshMemory(cell->memCellType)) {
        cout << string(indent, ' ') << " - Refresh Dynamic Energy = " << TO_JOULE(bank->refreshDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            cout << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			cout << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingRefreshDynamicEnergy) << endl;
		else
			cout << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingRefreshDynamicEnergy) << endl;
        cout << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.refreshDynamicEnergy) << " per subarray" << endl;
        cout << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << 
                TO_JOULE(bank->subarray.refreshDynamicEnergy - bank->subarray.mat.refreshDynamicEnergy
                         * bank->numActiveMatPerRow * bank->numActiveMatPerColumn) << endl;
        cout << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.refreshDynamicEnergy) << " per active mat" << endl;
        if (cell->memCellType == gcDRAM) {
			PrintGcDRAMRefreshEnergy(cout, indent, bank->subarray.mat, cell->memCellType);
		} else {
			cout << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy Per Row = " << TO_JOULE(bank->subarray.mat.rowDecoder.readDynamicEnergy) << endl;
			if (inputParameter->internalSensing)
				cout << string(indent, ' ') << "       |--- Senseamp Dynamic Energy Per Row    = " << TO_JOULE(bank->subarray.mat.senseAmp.readDynamicEnergy) << endl;
			cout << string(indent, ' ') << "       |--- Precharge Dynamic Energy Per Row   = " << TO_JOULE(bank->subarray.mat.precharger.readDynamicEnergy) << endl;
		}
    }

	cout << string(indent, ' ') << " - Leakage Power = " << TO_WATT(bank->leakage) << endl;
	cout << string(indent, ' ') << " -- Mat Leakage Power = " << TO_WATT(bank->subarray.mat.leakage) << endl;
	PrintAOSLeakageUpperBound(cout, indent, bank->subarray.mat);
	/* Mat Component Leakage Analysis */
	if (inputParameter->viewMatStats){
		cout << string(indent, ' ') << " -- Mat Leakage rowDecoder Power = " << TO_WATT(bank->subarray.mat.rowDecoder.leakage) << endl;
		cout << string(indent, ' ') << " -- Mat Leakage bitlineMuxDecoder Power = " << TO_WATT(bank->subarray.mat.bitlineMuxDecoder.leakage) << endl;
		cout << string(indent, ' ') << " -- Mat Leakage bitlineMux Power = " << TO_WATT(bank->subarray.mat.bitlineMux.leakage) << endl;
		cout << string(indent, ' ') << " -- Mat Leakage senseAmpMuxLev1 Power = " << TO_WATT(bank->subarray.mat.senseAmpMuxLev1.leakage) << endl;
		cout << string(indent, ' ') << " -- Mat Leakage senseAmpMuxLev2 Power = " << TO_WATT(bank->subarray.mat.senseAmpMuxLev2.leakage) << endl;
		cout << string(indent, ' ') << " -- Mat Leakage senseAmpMuxLev1Decoder Power = " << TO_WATT(bank->subarray.mat.senseAmpMuxLev1Decoder.leakage) << endl;
		cout << string(indent, ' ') << " -- Mat Leakage senseAmpMuxLev2Decoder Power = " << TO_WATT(bank->subarray.mat.senseAmpMuxLev2Decoder.leakage) << endl;
		cout << string(indent, ' ') << " -- Mat Leakage precharger Power = " << TO_WATT(bank->subarray.mat.precharger.leakage) << endl;
		cout << string(indent, ' ') << " -- Mat Leakage senseAmp Power = " << TO_WATT(bank->subarray.mat.senseAmp.leakage) << endl;
		cout << string(indent, ' ') << " END SUBARRAY LEAKAGE ANALYSIS "<< endl;
	}
	/* Mat Component Leakage Analysis END*/
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        cout << string(indent, ' ') << " |--- TSV Leakage              = " << TO_WATT(bank->tsvArray.leakage * (bank->stackedDieCount-1) * bank->tsvArray.numTotalBits) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        cout << string(indent, ' ') << " |--- TSV Leakage              = " << TO_WATT(bank->tsvArray.leakage * (bank->stackedDieCount-1) * bank->tsvArray.numTotalBits + bank->subarray.tsvArray.leakage * bank->numColumnSubArray * bank->numRowSubArray * bank->subarray.tsvArray.numTotalBits) << endl;
    }
	if (inputParameter->routingMode == h_tree){
		cout << string(indent, ' ') << " |--- H-Tree Leakage Power     = " << TO_WATT(bank->routingLeakage) << endl;
	} else
		cout << string(indent, ' ') << " |--- Non-H-Tree Leakage Power = " << TO_WATT(bank->routingLeakage) << endl;
	cout << string(indent, ' ') << " |--- SubArray Leakage Power        = " << TO_WATT(bank->subarray.leakage) << " per subarray" << endl;
    if (IsRefreshMemory(cell->memCellType)) {
        cout << string(indent, ' ') << " - Refresh Power = " << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime)) << endl;
    }
}

void Result::printToFile(int indent, const string &FileName) {
	std::ofstream outFile(FileName.c_str());
	if (!outFile.is_open()) {
		std::cerr << "Error opening file: " << FileName << std::endl;
		return;
	}
	printToStream(indent, outFile);
}

void Result::printToStream(int indent, ostream &outFile) {
	/* Deliberately shadow the process-wide exploration pointer with this snapshot's cell. */
	MemCell *const cell = cellTech;
	if (!cell) {
		cerr << "Result has no associated memory cell." << endl;
		return;
	}

    	outFile << string(indent, ' ') << endl;
    outFile << string(indent, ' ') << "=============" << endl;
    outFile << string(indent, ' ') << "   SUMMARY   " << endl;
    outFile << string(indent, ' ') << "=============" << endl;
    outFile << string(indent, ' ') << "Optimized for: " << printOptimizationTarget() << endl;
    // cellTech->PrintCell(indent);
	outFile << string(indent, ' ') << endl;
    outFile << string(indent, ' ') << "=============" << endl;
    outFile << string(indent, ' ') << "CONFIGURATION" << endl;
    outFile << string(indent, ' ') << "=============" << endl;
    if (bank->stackedDieCount > 1) {
        outFile << string(indent, ' ') << "Bank Organization: " << bank->numRowSubArray << " x " << bank->numColumnSubArray << " x " << bank->stackedDieCount << endl;
        outFile << string(indent, ' ') << " - Row Activation   : " << bank->numActiveSubArrayPerColumn << " / " << bank->numRowSubArray << " x 1" << endl;
        outFile << string(indent, ' ') << " - Column Activation: " << bank->numActiveSubArrayPerRow << " / " << bank->numColumnSubArray << " x 1" << endl;
    } else {
        outFile << string(indent, ' ') << "Bank Organization: " << bank->numRowSubArray << " x " << bank->numColumnSubArray << endl;
        outFile << string(indent, ' ') << " - Row Activation   : " << bank->numActiveSubArrayPerColumn << " / " << bank->numRowSubArray << endl;
        outFile << string(indent, ' ') << " - Column Activation: " << bank->numActiveSubArrayPerRow << " / " << bank->numColumnSubArray << endl;
    }
	outFile << string(indent, ' ') << "SubArray Organization: " << bank->numRowMat << " x " << bank->numColumnMat << endl;
	outFile << string(indent, ' ') << " - Row Activation   : " << bank->numActiveMatPerColumn << " / " << bank->numRowMat << endl;
	outFile << string(indent, ' ') << " - Column Activation: " << bank->numActiveMatPerRow << " / " << bank->numColumnMat << endl;
	outFile << string(indent, ' ') << " - Mat Size    : " << bank->subarray.mat.numRow << " Rows x " << bank->subarray.mat.numColumn << " Columns" << endl;
	PrintIntegratedModelLabels(outFile, indent, bank->subarray.mat, *cell);
	outFile << string(indent, ' ') << "Mux Level:" << endl;
	outFile << string(indent, ' ') << " - Senseamp Mux      : " << bank->muxSenseAmp << endl;
	outFile << string(indent, ' ') << " - Output Level-1 Mux: " << bank->muxOutputLev1 << endl;
	outFile << string(indent, ' ') << " - Output Level-2 Mux: " << bank->muxOutputLev2 << endl;
	if (inputParameter->designTarget == cache)
		outFile << string(indent, ' ') << " - One set is partitioned into " << bank->numRowPerSet << " rows" << endl;
	outFile << string(indent, ' ') << "Local Wire:" << endl;
	outFile << string(indent, ' ') << " - Wire Type : ";
	switch (localWire->wireType) {
	case local_aggressive:
		outFile << string(indent, ' ') << "Local Aggressive" << endl;
		break;
	case local_conservative:
		outFile << string(indent, ' ') << "Local Conservative" << endl;
		break;
	case semi_aggressive:
		outFile << string(indent, ' ') << "Semi-Global Aggressive" << endl;
		break;
	case semi_conservative:
		outFile << string(indent, ' ') << "Semi-Global Conservative" << endl;
		break;
	case global_aggressive:
		outFile << string(indent, ' ') << "Global Aggressive" << endl;
		break;
	case global_conservative:
		outFile << string(indent, ' ') << "Global Conservative" << endl;
		break;
	default:
		outFile << string(indent, ' ') << "DRAM Wire" << endl;
	}
	outFile << string(indent, ' ') << " - Repeater Type: ";
	switch (localWire->wireRepeaterType) {
	case repeated_none:
		outFile << string(indent, ' ') << "No Repeaters" << endl;
		break;
	case repeated_opt:
		outFile << string(indent, ' ') << "Fully-Optimized Repeaters" << endl;
		break;
	case repeated_5:
		outFile << string(indent, ' ') << "Repeaters with 5% Overhead" << endl;
		break;
	case repeated_10:
		outFile << string(indent, ' ') << "Repeaters with 10% Overhead" << endl;
		break;
	case repeated_20:
		outFile << string(indent, ' ') << "Repeaters with 20% Overhead" << endl;
		break;
	case repeated_30:
		outFile << string(indent, ' ') << "Repeaters with 30% Overhead" << endl;
		break;
	case repeated_40:
		outFile << string(indent, ' ') << "Repeaters with 40% Overhead" << endl;
		break;
	case repeated_50:
		outFile << string(indent, ' ') << "Repeaters with 50% Overhead" << endl;
		break;
	default:
		outFile << string(indent, ' ') << "Unknown" << endl;
	}
	outFile << string(indent, ' ') << " - Low Swing : ";
	if (localWire->isLowSwing)
		outFile << string(indent, ' ') << "Yes" << endl;
	else
		outFile << string(indent, ' ') << "No" << endl;
	outFile << string(indent, ' ') << "Global Wire:" << endl;
	outFile << string(indent, ' ') << " - Wire Type : ";
	switch (globalWire->wireType) {
	case local_aggressive:
		outFile << string(indent, ' ') << "Local Aggressive" << endl;
		break;
	case local_conservative:
		outFile << string(indent, ' ') << "Local Conservative" << endl;
		break;
	case semi_aggressive:
		outFile << string(indent, ' ') << "Semi-Global Aggressive" << endl;
		break;
	case semi_conservative:
		outFile << string(indent, ' ') << "Semi-Global Conservative" << endl;
		break;
	case global_aggressive:
		outFile << string(indent, ' ') << "Global Aggressive" << endl;
		break;
	case global_conservative:
		outFile << string(indent, ' ') << "Global Conservative" << endl;
		break;
	default:
		outFile << string(indent, ' ') << "DRAM Wire" << endl;
	}
	outFile << string(indent, ' ') << " - Repeater Type: ";
	switch (globalWire->wireRepeaterType) {
	case repeated_none:
		outFile << string(indent, ' ') << "No Repeaters" << endl;
		break;
	case repeated_opt:
		outFile << string(indent, ' ') << "Fully-Optimized Repeaters" << endl;
		break;
	case repeated_5:
		outFile << string(indent, ' ') << "Repeaters with 5% Overhead" << endl;
		break;
	case repeated_10:
		outFile << string(indent, ' ') << "Repeaters with 10% Overhead" << endl;
		break;
	case repeated_20:
		outFile << string(indent, ' ') << "Repeaters with 20% Overhead" << endl;
		break;
	case repeated_30:
		outFile << string(indent, ' ') << "Repeaters with 30% Overhead" << endl;
		break;
	case repeated_40:
		outFile << string(indent, ' ') << "Repeaters with 40% Overhead" << endl;
		break;
	case repeated_50:
		outFile << string(indent, ' ') << "Repeaters with 50% Overhead" << endl;
		break;
	default:
		outFile << string(indent, ' ') << "Unknown" << endl;
	}
	outFile << string(indent, ' ') << " - Low Swing : ";
	if (globalWire->isLowSwing)
		outFile << string(indent, ' ') << "Yes" << endl;
	else
		outFile << string(indent, ' ') << "No" << endl;
	outFile << string(indent, ' ') << "Buffer Design Style: ";
	switch (bank->areaOptimizationLevel) {
	case latency_first:
		outFile << string(indent, ' ') << "Latency-Optimized" << endl;
		break;
	case area_first:
		outFile << string(indent, ' ') << "Area-Optimized" << endl;
		break;
	default:	/* balance */
		outFile << string(indent, ' ') << "Balanced" << endl;
	}

	outFile << string(indent, ' ') << "=============" << endl;
    outFile << string(indent, ' ') << "   RESULT" << endl;
    outFile << string(indent, ' ') << "=============" << endl;

	outFile << string(indent, ' ') << "Area:" << endl;

	outFile << string(indent, ' ') << " - Total Area = " << TO_METER(bank->height) << " x " << TO_METER(bank->width)
			<< " = " << TO_SQM(bank->area) << endl;
	outFile << string(indent, ' ') << " |--- SubArray Area      = " << TO_METER(bank->subarray.height) << " x " << TO_METER(bank->subarray.width)
			<< " = " << TO_SQM(bank->subarray.area) << "   (" << cell->area * tech->featureSize * tech->featureSize
			* bank->capacity / bank->numRowSubArray / bank->numColumnSubArray / bank->subarray.area * 100 << "%)" << endl;
	outFile << string(indent, ' ') << " |--- Mat Area = " << TO_METER(bank->subarray.mat.height) << " x "
			<< TO_METER(bank->subarray.mat.width) << " = " << TO_SQM(bank->subarray.mat.area) << "   ("
			<< cell->area * tech->featureSize * tech->featureSize * bank->capacity / bank->numRowSubArray
			/ bank->numColumnSubArray / bank->numRowMat / bank->numColumnMat
			/ bank->subarray.mat.area * 100 << "%)" <<endl;
	
	/* Mat Area BreakDown */

	if (inputParameter->viewMatStats){
		outFile << string(indent, ' ') << " |--- Mat rowDecoder Area = " << TO_SQM(2*bank->subarray.mat.rowDecoder.area) << endl;
		if (cell->memCellType == gcDRAM)
			outFile << string(indent, ' ') << " |--- Mat gcDRAM Read Row Decoder Area = " << TO_SQM(bank->subarray.mat.gcRowDecoder.area) << endl;
		outFile << string(indent, ' ') << " |--- Mat bitlineMuxDecoder Area = " << TO_SQM(2*bank->subarray.mat.bitlineMuxDecoder.area) << endl;
		outFile << string(indent, ' ') << " |--- Mat bitlineMux Area = " << TO_SQM(bank->subarray.mat.bitlineMux.area) << endl;
		outFile << string(indent, ' ') << " |--- Mat senseAmpMuxLev1 Area = " << TO_SQM(bank->subarray.mat.senseAmpMuxLev1.area) << endl;
		outFile << string(indent, ' ') << " |--- Mat senseAmpMuxLev2 Area = " << TO_SQM(bank->subarray.mat.senseAmpMuxLev2.area) << endl;
		outFile << string(indent, ' ') << " |--- Mat senseAmpMuxLev1Decoder Area = " << TO_SQM(bank->subarray.mat.senseAmpMuxLev1Decoder.area) << endl;
		outFile << string(indent, ' ') << " |--- Mat senseAmpMuxLev2Decoder Area = " << TO_SQM(bank->subarray.mat.senseAmpMuxLev2Decoder.area) << endl;
		outFile << string(indent, ' ') << " |--- Mat precharger Area = " << TO_SQM(2*bank->subarray.mat.precharger.area) << endl;
		if (cell->memCellType == gcDRAM)
			outFile << string(indent, ' ') << " |--- Mat gcDRAM Write Charger Area = " << TO_SQM(bank->subarray.mat.writecharger.area) << endl;
		outFile << string(indent, ' ') << " |--- Mat senseAmp Area =" << TO_SQM(bank->subarray.mat.senseAmp.area) << endl;
		outFile << string(indent, ' ') << " |--- Mat MIV Area = " << TO_SQM(bank->subarray.mat.tsvArray.area) << endl;
	}

	/*Mat Area BreakDown End*/
	
	if (inputParameter->monolithic3DMat){
		PrintM3DDetails(outFile, indent, bank->subarray.mat);
	}
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        outFile << string(indent, ' ') << " |--- TSV Area      = " << TO_SQM(bank->tsvArray.area) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        double totalTSVArea = bank->tsvArray.area + bank->subarray.tsvArray.area
                              * bank->numColumnSubArray * bank->numRowSubArray;
        double areaLogicLayer = bank->subarray.areaAllLogicBlocks * bank->numColumnSubArray
                              * bank->numRowSubArray;

        outFile << string(indent, ' ') << " |--- TSV Area      = " << TO_SQM(totalTSVArea) << endl;
        outFile << string(indent, ' ') << " |--- Logic Layer Area = " << TO_SQM(areaLogicLayer) << endl;
    }
	outFile << string(indent, ' ') << " - Area Efficiency = " << cell->area * tech->featureSize * tech->featureSize
			* bank->capacity / bank->area * 100 << "%" << endl;

	outFile << string(indent, ' ') << "Timing:" << endl;

	outFile << string(indent, ' ') << " -  Read Latency = " << TO_SECOND(bank->readLatency) << endl;
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        double totalTSVLatency = (bank->tsvArray.readLatency + bank->tsvArray.writeLatency) * (bank->stackedDieCount-1);
        outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(totalTSVLatency) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        double totalTSVLatency = bank->tsvArray.readLatency * (bank->stackedDieCount-1)
                               + bank->subarray.tsvArray.writeLatency * (bank->stackedDieCount-1);

        outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(totalTSVLatency) << endl;
    }
	if (inputParameter->routingMode == h_tree)
		outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingReadLatency) << endl;
	else
		outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingReadLatency) << endl;
	outFile << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.readLatency) << endl;
	outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
	outFile << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.readLatency) << endl;
	PrintIntegratedTimingDetails(outFile, indent, bank->subarray.mat, cell->memCellType);
	if(cell->memCellType != gcDRAM) { 
		outFile << string(indent, ' ') << "       |--- Row Decoder Latency = " << TO_SECOND(bank->subarray.mat.rowDecoder.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Bitline Latency     = " << TO_SECOND(bank->subarray.mat.bitlineDelay) << endl;
		if (inputParameter->internalSensing)
			outFile << string(indent, ' ') << "       |--- Senseamp Latency    = " << TO_SECOND(bank->subarray.mat.senseAmp.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Precharge Latency   = " << TO_SECOND(bank->subarray.mat.precharger.readLatency) << endl;
	}
	else {
		outFile << string(indent, ' ') << "       |--- Read Row Decoder Latency = " << TO_SECOND(bank->subarray.mat.gcRowDecoder.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Write Row Decoder Latency = " << TO_SECOND(bank->subarray.mat.rowDecoder.readLatency) << endl;
		//outFile << string(indent, ' ') << "       |--- Write Level Shifter Latency = " << TO_SECOND(bank->subarray.mat.write_levelshifter.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Bitline Latency     = " << TO_SECOND(bank->subarray.mat.readBitlineDelay) << endl;
		if (inputParameter->internalSensing)
			outFile << string(indent, ' ') << "       |--- Senseamp Latency    = " << TO_SECOND(bank->subarray.mat.senseAmp.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Precharge Latency   = " << TO_SECOND(bank->subarray.mat.precharger.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Write Bitline Latency     = " << TO_SECOND(bank->subarray.mat.writeBitlineDelay) << endl;
		outFile << string(indent, ' ') << "       |--- Write Drive Latency   = " << TO_SECOND(bank->subarray.mat.writecharger.readLatency) << endl;
	}

	outFile << string(indent, ' ') << "       |--- Mux Latency         = " << TO_SECOND(bank->subarray.mat.bitlineMux.readLatency
													+ bank->subarray.mat.senseAmpMuxLev1.readLatency
													+ bank->subarray.mat.senseAmpMuxLev2.readLatency) << endl;
	if (bank->subarray.memoryType == MemoryType::tag && bank->subarray.internalSenseAmp)
		outFile << string(indent, ' ') << "    |--- Comparator Latency  = " << TO_SECOND(bank->subarray.comparator.readLatency) << endl;

	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM ||
			(cell->memCellType == memristor && (cell->accessType == CMOS_access || cell->accessType == BJT_access))) {
		outFile << string(indent, ' ') << " - RESET Latency = " << TO_SECOND(bank->resetLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.resetLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.resetLatency) << endl;
		outFile << string(indent, ' ') << "       |--- RESET Pulse Duration = " << TO_SECOND(cell->resetPulse) << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Latency  = " << TO_SECOND(bank->subarray.mat.rowDecoder.writeLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Charge Latency   = " << TO_SECOND(bank->subarray.mat.chargeLatency) << endl;
		outFile << string(indent, ' ') << " - SET Latency   = " << TO_SECOND(bank->setLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.setLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.setLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.setLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.setLatency) << endl;
		outFile << string(indent, ' ') << "       |--- SET Pulse Duration   = " << TO_SECOND(cell->setPulse) << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Latency  = " << TO_SECOND(bank->subarray.mat.rowDecoder.writeLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Charger Latency      = " << TO_SECOND(bank->subarray.mat.chargeLatency) << endl;
	} else if (cell->memCellType == SLCNAND) {
		outFile << string(indent, ' ') << " - Erase Latency = " << TO_SECOND(bank->resetLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.resetLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingResetLatency) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.resetLatency) << endl;
		outFile << string(indent, ' ') << " - Programming Latency   = " << TO_SECOND(bank->setLatency) << endl;
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingSetLatency) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.setLatency) << endl;
	} else {
		outFile << string(indent, ' ') << " - Write Latency = " << TO_SECOND(bank->writeLatency) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingWriteLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingWriteLatency) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.writeLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.writeLatency) << endl;
		if (cell->memCellType == MRAM)
			outFile << string(indent, ' ') << "       |--- Write Pulse Duration = " << TO_SECOND(cell->resetPulse) << endl;	// MRAM reset/set is equal
		outFile << string(indent, ' ') << "       |--- Row Decoder Latency = " << TO_SECOND(bank->subarray.mat.rowDecoder.writeLatency) << endl;
		//if(cell->memCellType == gcDRAM) outFile << string(indent, ' ') << "       |--- Level Shifter Latency = " << TO_SECOND(bank->subarray.mat.write_levelshifter.readLatency) << endl;
		outFile << string(indent, ' ') << "       |--- Charge Latency      = " << TO_SECOND(bank->subarray.mat.chargeLatency) << endl;
	}
    if (IsRefreshMemory(cell->memCellType)) {
        outFile << string(indent, ' ') << " - Refresh Latency = " << TO_SECOND(bank->refreshLatency) << endl;
        if ((bank->stackedDieCount > 1 && bank->partitionGranularity == 0)) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Latency    = " << TO_SECOND(bank->tsvArray.writeLatency * (bank->stackedDieCount-1)) << endl;
        } if (bank->subarray.mat.stackedMemTiers) {
			//outFile << string(indent, ' ') << " |--- MIV Latency    = " << TO_SECOND(bank->subarray.mat.tsvArray.writeLatency * (bank->subarray.mat.stackedMemTiers)) << endl;
			//outFile << string(indent, ' ') << " |--- BL Res    = " << TO_SECOND(bank->subarray.mat.resBitline) << endl;
			//outFile << string(indent, ' ') << " |--- BL Cap    = " << TO_SECOND(bank->subarray.mat.capBitline) << endl;
		}
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Latency = " << TO_SECOND(bank->routingRefreshLatency) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Latency = " << TO_SECOND(bank->routingRefreshLatency) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Latency    = " << TO_SECOND(bank->subarray.refreshLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Latency = " << TO_SECOND(bank->subarray.predecoderLatency) << endl;
		outFile << string(indent, ' ') << "    |--- Mat Latency   = " << TO_SECOND(bank->subarray.mat.refreshLatency) << endl;
    }

	double readBandwidth = getReadBandwidth();
	//outFile << "DEBUGGING: bank blockSize - " << bank->blockSize << endl;
	//outFile << "DEBUGGING: mat readLatency - " << TO_SECOND(bank->subarray.mat.readLatency) << ", decoder readLatency - " << TO_SECOND(bank->subarray.mat.rowDecoder.readLatency) << ", precharger latency - " << TO_SECOND(bank->subarray.mat.precharger.readLatency) << endl;
	outFile << string(indent, ' ') << " - Read Bandwidth  = " << TO_BPS(readBandwidth) << endl;
	double writeBandwidth = getWriteBandwidth();
	outFile << string(indent, ' ') << " - Write Bandwidth = " << TO_BPS(writeBandwidth) << endl;

	outFile << string(indent, ' ') << "Power:" << endl;

	outFile << string(indent, ' ') << " -  Read Dynamic Energy = " << TO_JOULE(bank->readDynamicEnergy) << endl;
    if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
        // address and control bit dynamics + read data output dynamic
        outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits + bank->tsvArray.readDynamicEnergy * bank->tsvArray.numDataBits * (bank->stackedDieCount-1)) << endl;
    } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
        // address bit dynamic + control bit dynamic + read data dynamic
        // TODO: revisit this
        outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->subarray.tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * (bank->subarray.tsvArray.numAccessBits) + bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->stackedDieCount + bank->tsvArray.readDynamicEnergy * bank->blockSize * (bank->stackedDieCount-1)) << endl;
    }
	if (inputParameter->routingMode == h_tree)
		outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingReadDynamicEnergy) << endl;
	else
		outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingReadDynamicEnergy) << endl;
	outFile << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.readDynamicEnergy) << " per subarray" << endl;
	outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.readDynamicEnergy - bank->subarray.mat.readDynamicEnergy
														* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
														<< endl;
	outFile << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.readDynamicEnergy) << " per active mat" << endl;\
	if (cell->memCellType != gcDRAM)
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.readDynamicEnergy) << endl;
	else
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.gcRowDecoder.readDynamicEnergy) << endl;
	outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.readDynamicEnergy
													+ bank->subarray.mat.senseAmpMuxLev1Decoder.readDynamicEnergy
													+ bank->subarray.mat.senseAmpMuxLev2Decoder.readDynamicEnergy) << endl;
	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM || cell->memCellType == MRAM || cell->memCellType == memristor ) {
		outFile << string(indent, ' ') << "       |--- Bitline & Cell Read Energy = " << TO_JOULE(bank->subarray.mat.cellReadEnergy) << endl;
	}
	if (inputParameter->internalSensing)
		outFile << string(indent, ' ') << "       |--- Senseamp Dynamic Energy    = " << TO_JOULE(bank->subarray.mat.senseAmp.readDynamicEnergy) << endl;
	outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.readDynamicEnergy
													+ bank->subarray.mat.senseAmpMuxLev1.readDynamicEnergy
													+ bank->subarray.mat.senseAmpMuxLev2.readDynamicEnergy) << endl;
	outFile << string(indent, ' ') << "       |--- Precharge Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.precharger.readDynamicEnergy) << endl;
	PrintGcDRAMReadEnergy(outFile, indent, bank->subarray.mat, cell->memCellType);

	if (cell->memCellType == PCRAM || cell->memCellType == FBRAM ||
			(cell->memCellType == memristor && (cell->accessType == CMOS_access || cell->accessType == BJT_access))) {
		outFile << string(indent, ' ') << " - RESET Dynamic Energy = " << TO_JOULE(bank->resetDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.resetDynamicEnergy) << " per subarray" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Cell RESET Dynamic Energy  = " << TO_JOULE(bank->subarray.mat.cellResetEnergy) << endl;
		outFile << string(indent, ' ') << " - SET Dynamic Energy = " << TO_JOULE(bank->setDynamicEnergy) << endl;
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.setDynamicEnergy) << " per subarray" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Cell SET Dynamic Energy    = " << TO_JOULE(bank->subarray.mat.cellSetEnergy) << endl;
	} else if (cell->memCellType == SLCNAND) {
		outFile << string(indent, ' ') << " - Erase Dynamic Energy = " << TO_JOULE(bank->resetDynamicEnergy) << " per block" << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.resetDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingResetDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.resetDynamicEnergy) << " per subarray" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " - Programming Dynamic Energy = " << TO_JOULE(bank->setDynamicEnergy) << " per page" << endl;
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingSetDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.setDynamicEnergy) << " per subarray" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
	} else {
		outFile << string(indent, ' ') << " - Write Dynamic Energy = " << TO_JOULE(bank->writeDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numAccessBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingWriteDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingWriteDynamicEnergy) << endl;
		outFile << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.writeDynamicEnergy) << " per subarray" << endl;
		outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << TO_JOULE(bank->subarray.writeDynamicEnergy - bank->subarray.mat.writeDynamicEnergy
															* bank->numActiveMatPerRow * bank->numActiveMatPerColumn)
															<< endl;
		outFile << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.writeDynamicEnergy) << " per active mat" << endl;
		outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.rowDecoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Decoder Dynamic Energy = " << TO_JOULE(bank->subarray.mat.bitlineMuxDecoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1Decoder.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2Decoder.writeDynamicEnergy) << endl;
		outFile << string(indent, ' ') << "       |--- Mux Dynamic Energy         = " << TO_JOULE(bank->subarray.mat.bitlineMux.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev1.writeDynamicEnergy
														+ bank->subarray.mat.senseAmpMuxLev2.writeDynamicEnergy) << endl;
		if (cell->memCellType == MRAM) {
			outFile << string(indent, ' ') << "       |--- Bitline & Cell Write Energy= " << TO_JOULE(bank->subarray.mat.cellResetEnergy) << endl;
		}
	}
	PrintGcDRAMWriteEnergy(outFile, indent, bank->subarray.mat, cell->memCellType);
    if (IsRefreshMemory(cell->memCellType)) {
        outFile << string(indent, ' ') << " - Refresh Dynamic Energy = " << TO_JOULE(bank->refreshDynamicEnergy) << endl;
        if (bank->stackedDieCount > 1 && bank->partitionGranularity == 0) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits) << endl;
        } else if (bank->stackedDieCount > 1 && bank->partitionGranularity == 1) {
            outFile << string(indent, ' ') << " |--- TSV Dynamic Energy    = " << TO_JOULE(bank->tsvArray.writeDynamicEnergy * (bank->stackedDieCount-1) * bank->tsvArray.numReadBits) << endl;
        }
		if (inputParameter->routingMode == h_tree)
			outFile << string(indent, ' ') << " |--- H-Tree Dynamic Energy = " << TO_JOULE(bank->routingRefreshDynamicEnergy) << endl;
		else
			outFile << string(indent, ' ') << " |--- Non-H-Tree Dynamic Energy = " << TO_JOULE(bank->routingRefreshDynamicEnergy) << endl;
        outFile << string(indent, ' ') << " |--- SubArray Dynamic Energy    = " << TO_JOULE(bank->subarray.refreshDynamicEnergy) << " per subarray" << endl;
        outFile << string(indent, ' ') << "    |--- Predecoder Dynamic Energy = " << 
                TO_JOULE(bank->subarray.refreshDynamicEnergy - bank->subarray.mat.refreshDynamicEnergy
                         * bank->numActiveMatPerRow * bank->numActiveMatPerColumn) << endl;
        outFile << string(indent, ' ') << "    |--- Mat Dynamic Energy   = " << TO_JOULE(bank->subarray.mat.refreshDynamicEnergy) << " per active mat" << endl;
		if (cell->memCellType == gcDRAM) {
			PrintGcDRAMRefreshEnergy(outFile, indent, bank->subarray.mat, cell->memCellType);
		} else {
			outFile << string(indent, ' ') << "       |--- Row Decoder Dynamic Energy Per Row = " << TO_JOULE(bank->subarray.mat.rowDecoder.readDynamicEnergy) << endl;
			if (inputParameter->internalSensing)
				outFile << string(indent, ' ') << "       |--- Senseamp Dynamic Energy Per Row    = " << TO_JOULE(bank->subarray.mat.senseAmp.readDynamicEnergy) << endl;
			outFile << string(indent, ' ') << "       |--- Precharge Dynamic Energy Per Row   = " << TO_JOULE(bank->subarray.mat.precharger.readDynamicEnergy) << endl;
		}
	}
	outFile << string(indent, ' ') << " - Leakage Power = " << TO_WATT(bank->leakage) << endl;
	outFile << string(indent, ' ') << " -- Mat Leakage Power = " << TO_WATT(bank->subarray.mat.leakage) << endl;
	PrintAOSLeakageUpperBound(outFile, indent, bank->subarray.mat);
	if (IsRefreshMemory(cell->memCellType)) {
		outFile << string(indent, ' ') << " - Refresh Power = "
				<< TO_WATT(bank->refreshDynamicEnergy / cell->retentionTime) << endl;
	}
}

void Result::printAsCache(Result &tagResult, CacheAccessMode cacheAccessMode) {
	/* Cache-level refresh behavior follows the saved data-array cell. */
	MemCell *const cell = cellTech;
	if (!cell) {
		cerr << "Result has no associated memory cell." << endl;
		return;
	}
	if (bank->memoryType != MemoryType::data || tagResult.bank->memoryType != MemoryType::tag) {
		cout << "This is not a valid cache configuration." << endl;
		return;
	} else {
		double cacheHitLatency, cacheMissLatency, cacheWriteLatency;
		double cacheHitDynamicEnergy, cacheMissDynamicEnergy, cacheWriteDynamicEnergy;
		double cacheLeakage;
		double cacheArea;
		if (cacheAccessMode == normal_access_mode) {
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;		/* only the tag access latency */
			cacheHitLatency = MAX(tagResult.bank->readLatency, bank->subarray.readLatency);	/* access tag and activate data row in parallel */
			cacheHitLatency += bank->subarray.mat.columnDecoderLatency;		/* add column decoder latency after hit signal arrives */
			cacheHitLatency += bank->readLatency - bank->subarray.readLatency;	/* H-tree in and out latency */
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);	/* Data and tag are written in parallel */
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no subarrayter what tag is always accessed */
			cacheMissDynamicEnergy += bank->readDynamicEnergy;	/* data is also partially accessed, TO-DO: not accurate here */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		} else if (cacheAccessMode == fast_access_mode) {
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;
			cacheHitLatency = MAX(tagResult.bank->readLatency, bank->readLatency);
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no subarrayter what tag is always accessed */
			cacheMissDynamicEnergy += bank->readDynamicEnergy;	/* data is also partially accessed, TO-DO: not accurate here */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		} else {		/* sequential access */
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;
			cacheHitLatency = tagResult.bank->readLatency + bank->readLatency;
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no subarrayter what tag is always accessed */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		}
		/* Calculate leakage */
		cacheLeakage = tagResult.bank->leakage + bank->leakage;
		/* Calculate area */
		cacheArea = tagResult.bank->area + bank->area;	/* TO-DO: simply add them together here */

		/* start printing */
		cout << endl << "=======================" << endl << "CACHE DESIGN -- SUMMARY" << endl << "=======================" << endl;
		cout << "Access Mode: ";
		switch (cacheAccessMode) {
		case normal_access_mode:
			cout << "Normal" << endl;
			break;
		case fast_access_mode:
			cout << "Fast" << endl;
			break;
		default:	/* sequential */
			cout << "Sequential" << endl;
		}
		cout << "Area:" << endl;
		cout << " - Total Area = " << cacheArea * 1e6 << "mm^2" << endl;
		cout << " |--- Data Array Area = " << bank->height * 1e6 << "um x " << bank->width * 1e6 << "um = " << bank->area * 1e6 << "mm^2" << endl;
		cout << " |--- Tag Array Area  = " << tagResult.bank->height * 1e6 << "um x " << tagResult.bank->width * 1e6 << "um = " << tagResult.bank->area * 1e6 << "mm^2" << endl;
		cout << "Timing:" << endl;
		cout << " - Cache Hit Latency   = " << cacheHitLatency * 1e9 << "ns" << endl;
		cout << " - Cache Miss Latency  = " << cacheMissLatency * 1e9 << "ns" << endl;
		cout << " - Cache Write Latency = " << cacheWriteLatency * 1e9 << "ns" << endl;
		if(inputParameter->quantize) {
			cout << "Cycle Timing:" << endl;
			cout << " - Clock Frequency    = " << inputParameter->clockFreq / 1e6 << "MHz" << endl;
			cout << " - Cache Hit Cycles   = " << ceil(cacheHitLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Miss Cycles  = " << ceil(cacheMissLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Write Cycles = " << ceil(cacheWriteLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Data SubArray Read Cycles  = " << ceil(bank->subarray.readLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Data SubArray Write Cycles = " << ceil(bank->subarray.writeLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Data Mat Read Cycles  = " << ceil(bank->subarray.mat.readLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Data Mat Write Cycles = " << ceil(bank->subarray.mat.writeLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Tag SubArray Read Cycles  = " << ceil(tagResult.bank->subarray.readLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Tag SubArray Write Cycles = " << ceil(tagResult.bank->subarray.writeLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Tag Mat Read Cycles  = " << ceil(tagResult.bank->subarray.mat.readLatency * inputParameter->clockFreq) << " cycles" << endl;
			cout << " - Cache Tag Mat Write Cycles = " << ceil(tagResult.bank->subarray.mat.writeLatency * inputParameter->clockFreq) << " cycles" << endl;
		
		
			cout << "-------------------------------------------------\n" << endl;
			cout << "Usable GEM5 Command: (<string> indicates user inputed parameter)" << endl;
			cout << "./build/X86_MERSI_Three_Level/gem5.opt --outdir=<output_directory>  configs/example/syscall_emulation.py --cmd <executable_path> --options=<executable_cmd_line_options>--ruby -n <num_cores> --mem-size <DRAM_capacity>GB --mem-type <DRAM_type> --l0i_size <L1I_Capacity_Per_Core>kB --l0d_size <L1D_Capacity_Per_Core>kB --l0i_assoc <L1I_Associativity> --l0d_assoc <L1D_Associativity> --l1d_size <L2_Capacity_Per_Core>kB --l1d_assoc <L2_Associativity> --cpu-type <CPU Type> ";
			
			// helper: convert (seconds * Hz) -> integer cycles
			auto cycles = [&](double seconds) -> uint64_t {
			    const double roundedCycles = std::ceil(seconds * inputParameter->clockFreq);
			    if (!std::isfinite(roundedCycles) || roundedCycles <= 0)
			        return 0;
			    if (roundedCycles >= static_cast<double>(std::numeric_limits<uint64_t>::max()))
			        return std::numeric_limits<uint64_t>::max();
			    return static_cast<uint64_t>(roundedCycles);
			};
			const bool refreshEnabled = IsRefreshMemory(cell->memCellType);
			const uint64_t refreshGroupCount = static_cast<uint64_t>(bank->numRowMat)
					* static_cast<uint64_t>(inputParameter->monolithic3DMat
							? bank->subarray.mat.stackedMemTiers : 1);
			
			// NS-Cache Derived Parameteric Outputs
			cout << "--l2_assoc " << inputParameter->associativity << " ";
			cout << "--cacheline_size " << inputParameter->wordWidth << " ";
			cout << "--sys-clock " << (inputParameter->clockFreq / 1e9) << "GHz ";
			cout << "--l2_size " << (bank->capacity / 1024 / 1024 / 8) << "MB ";
			
			cout << "--l2_data_hit_latency "   << cycles(cacheHitLatency)   << " ";
			cout << "--l2_data_miss_latency "  << cycles(cacheMissLatency)  << " ";
			cout << "--l2_data_write_latency " << cycles(cacheWriteLatency) << " ";
			
			if (refreshEnabled) {
			    cout << "--l2_refresh_period "
			         << cycles(cell->retentionTime / refreshGroupCount) << " ";
			} else {
			    // 1e20 will overflow uint64_t; use a clear integer sentinel instead
			    cout << "--l2_refresh_period " << std::numeric_limits<uint64_t>::max() << " ";
			}
			
			if (refreshEnabled) {
			    /* gem5 models one blocking refresh event per emitted period.  The
			     * bank value is the aggregate serialized sweep for one vertical
			     * MAT/tier group: it includes DRAM restoration and, for gcDRAM,
			     * both read and write paths.  Do not substitute response latency. */
			    cout << "--l2_refresh_latency " << cycles(bank->refreshLatency) << " ";
			} else {
			    cout << "--l2_refresh_latency " << 0 << " ";
			}
			
			cout << "--l2_refresh_enabled " << refreshEnabled << " ";
			
			cout << "--data_read_latency "  << cycles(bank->subarray.readLatency) << " ";
			cout << "--data_write_latency " << cycles(bank->subarray.writeLatency) << " ";
			cout << "--tag_read_latency "   << cycles(tagResult.bank->subarray.readLatency) << " ";
			cout << "--tag_write_latency "  << cycles(tagResult.bank->subarray.writeLatency) << " ";
			
			// Calculate the serialized latency depending on access mode
			if (cacheAccessMode == normal_access_mode) {
			    const uint64_t tagCyc  = cycles(tagResult.bank->readLatency + (bank->readLatency/2 - bank->subarray.readLatency));
			    const uint64_t dataCyc = cycles(bank->readLatency/2 + bank->subarray.readLatency);
			    const uint64_t serial  = (tagCyc + 1 > dataCyc) ? (tagCyc + 1 - dataCyc) : 0;
			    cout << "--serial_latency " << serial << " \n\n";
			} else if (cacheAccessMode == sequential_access_mode) {
			    cout << "--serial_latency " << (cycles(cacheMissLatency) + 1) << " \n\n";
			} else {
			    cout << "--serial_latency " << 0 << " \n\n";
			}


			cout << "-------------------------------------------------\n" << endl;
		}
        if (IsRefreshMemory(cell->memCellType)) {
            cout << " - Cache Refresh Latency = " << MAX(tagResult.bank->refreshLatency, bank->refreshLatency) * 1e6 << "us per bank" << endl;
            cout << " - Cache Availability = " << ((cell->retentionTime - MAX(tagResult.bank->refreshLatency, bank->refreshLatency)) / cell->retentionTime) * 100.0 << "%" << endl;
        }
		cout << "Power:" << endl;
		cout << " - Cache Hit Dynamic Energy   = " << cacheHitDynamicEnergy * 1e9 << "nJ per access" << endl;
		cout << " - Cache Miss Dynamic Energy  = " << cacheMissDynamicEnergy * 1e9 << "nJ per access" << endl;
		cout << " - Cache Write Dynamic Energy = " << cacheWriteDynamicEnergy * 1e9 << "nJ per access" << endl;
        if (IsRefreshMemory(cell->memCellType)) {
            cout << " - Cache Refresh Dynamic Energy = " << (tagResult.bank->refreshDynamicEnergy + bank->refreshDynamicEnergy) * 1e9 << "nJ per bank" << endl;
        }
		cout << " - Cache Total Leakage Power  = " << cacheLeakage * 1e3 << "mW" << endl;
		cout << " |--- Cache Data Array Leakage Power = " << bank->leakage * 1e3 << "mW" << endl;
		cout << " |--- Cache Tag Array Leakage Power  = " << tagResult.bank->leakage * 1e3 << "mW" << endl;
        if (IsRefreshMemory(cell->memCellType)) {
            cout << " - Cache Refresh Power = " << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime)) << " per bank" << endl;
        }
        if (inputParameter->printLevel > 0) {
            cout << endl << "CACHE DATA ARRAY DETAILS";
            print(4);
            cout << endl << "CACHE TAG ARRAY DETAILS";
            tagResult.print(4);
        }
	}
}

void Result::printAsCacheToFile(Result &tagResult, CacheAccessMode cacheAccessMode, const string &FileName) {
	/* Cache-level refresh behavior follows the saved data-array cell. */
	MemCell *const cell = cellTech;
	if (!cell) {
		cerr << "Result has no associated memory cell." << endl;
		return;
	}
    // Attempt to open the file
    ofstream outFile(FileName.c_str());
    if (!outFile.is_open()) {
        cerr << "Error opening file: " << FileName << endl;
        return;
    }

    if (bank->memoryType != MemoryType::data || tagResult.bank->memoryType != MemoryType::tag) {
        outFile << "This is not a valid cache configuration." << endl;
        outFile.close();
        return;
    } else {
	    double cacheHitLatency = 0;
	    double cacheMissLatency = 0;
	    double cacheWriteLatency = 0;
	    double cacheHitDynamicEnergy = 0;
	    double cacheMissDynamicEnergy = 0;
	    double cacheWriteDynamicEnergy = 0;
	    double cacheLeakage = 0;
	    double cacheArea = 0;

	    if (cacheAccessMode == normal_access_mode) {
	        cacheMissLatency = tagResult.bank->readLatency;
	        cacheHitLatency = MAX(tagResult.bank->readLatency, bank->subarray.readLatency);
	        cacheHitLatency += bank->subarray.mat.columnDecoderLatency;
	        cacheHitLatency += bank->readLatency - bank->subarray.readLatency;
	        cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);

	        cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
	        cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
	        cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
	    } else if (cacheAccessMode == fast_access_mode) {
	        cacheMissLatency = tagResult.bank->readLatency;
	        cacheHitLatency = MAX(tagResult.bank->readLatency, bank->readLatency);
	        cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);

	        cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
	        cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
	        cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
	    } else { // sequential access
	        cacheMissLatency = tagResult.bank->readLatency;
	        cacheHitLatency = tagResult.bank->readLatency + bank->readLatency;
	        cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);

	        cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;
	        cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
	        cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
	    }

	    cacheLeakage = tagResult.bank->leakage + bank->leakage;
	    cacheArea = tagResult.bank->area + bank->area;

        // Now write to the file instead of the console
        outFile << endl
                << "=======================" << endl
                << "CACHE DESIGN -- SUMMARY" << endl
                << "=======================" << endl;
        outFile << "Access Mode: ";
        switch (cacheAccessMode) {
            case normal_access_mode:
                outFile << "Normal" << endl;
                break;
            case fast_access_mode:
                outFile << "Fast" << endl;
                break;
            default: // sequential
                outFile << "Sequential" << endl;
        }

        // Area
        outFile << "Area:" << endl;
        outFile << " - Total Area = " << cacheArea * 1e6 << "mm^2" << endl;
        outFile << " |--- Data Array Area = " << bank->height * 1e6 << "um x "
                << bank->width * 1e6 << "um = " << bank->area * 1e6 << "mm^2" << endl;
		outFile << " |--- Tag Array Area  = " << tagResult.bank->height * 1e6 << "um x "
				<< tagResult.bank->width * 1e6 << "um = " << tagResult.bank->area * 1e6 << "mm^2" << endl;

        // Timing
        outFile << "Timing:" << endl;
        outFile << " - Cache Hit Latency   = " << cacheHitLatency * 1e9 << "ns" << endl;
        outFile << " - Cache Miss Latency  = " << cacheMissLatency * 1e9 << "ns" << endl;
        outFile << " - Cache Write Latency = " << cacheWriteLatency * 1e9 << "ns" << endl;
		if(inputParameter->quantize) {
			outFile << "Cycle Timing:" << endl;
			outFile << " - Clock Frequency    = " << inputParameter->clockFreq / 1e6 << "MHz" << endl;
			outFile << " - Cache Hit Cycles   = " << ceil(cacheHitLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Miss Cycles  = " << ceil(cacheMissLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Write Cycles = " << ceil(cacheWriteLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Data SubArray Read Cycles  = " << ceil(bank->subarray.readLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Data SubArray Write Cycles = " << ceil(bank->subarray.writeLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Data Mat Read Cycles  = " << ceil(bank->subarray.mat.readLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Data Mat Write Cycles = " << ceil(bank->subarray.mat.writeLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Tag SubArray Read Cycles  = " << ceil(tagResult.bank->subarray.readLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Tag SubArray Write Cycles = " << ceil(tagResult.bank->subarray.writeLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Tag Mat Read Cycles  = " << ceil(tagResult.bank->subarray.mat.readLatency * inputParameter->clockFreq) << " cycles" << endl;
			outFile << " - Cache Tag Mat Write Cycles = " << ceil(tagResult.bank->subarray.mat.writeLatency * inputParameter->clockFreq) << " cycles" << endl;
		}
		if (IsRefreshMemory(cell->memCellType)) {
			const double cacheRefreshLatency = MAX(tagResult.bank->refreshLatency, bank->refreshLatency);
	        outFile << " - Cache Refresh Latency = "
	                << cacheRefreshLatency * 1e6
	                << "us per bank" << endl;
	        outFile << " - Cache Availability = "
	                << ((cell->retentionTime - cacheRefreshLatency) /
	                    cell->retentionTime) *
	                       100.0
                    << "%" << endl;
        }

        // Power
        outFile << "Power:" << endl;
        outFile << " - Cache Hit Dynamic Energy   = " << cacheHitDynamicEnergy * 1e9
                << "nJ per access" << endl;
        outFile << " - Cache Miss Dynamic Energy  = " << cacheMissDynamicEnergy * 1e9
                << "nJ per access" << endl;
        outFile << " - Cache Write Dynamic Energy = " << cacheWriteDynamicEnergy * 1e9
                << "nJ per access" << endl;
	    if (IsRefreshMemory(cell->memCellType)) {
	        outFile << " - Cache Refresh Dynamic Energy = "
	                << (tagResult.bank->refreshDynamicEnergy + bank->refreshDynamicEnergy) * 1e9
	                << "nJ per bank" << endl;
	    }
	    outFile << " - Cache Total Leakage Power  = " << cacheLeakage * 1e3 << "mW" << endl;
	    outFile << " |--- Cache Data Array Leakage Power = " << bank->leakage * 1e3 << "mW" << endl;
		outFile << " |--- Cache Tag Array Leakage Power  = " << tagResult.bank->leakage * 1e3 << "mW" << endl;
        if (IsRefreshMemory(cell->memCellType)) {
            outFile << " - Cache Refresh Power = "
                    << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime))
                    << " per bank" << endl;
        }

        // Print details if needed
        if (inputParameter->printLevel > 0) {
            outFile << endl << "CACHE DATA ARRAY DETAILS";
			printToStream(4, outFile);
			outFile << endl << "CACHE TAG ARRAY DETAILS";
			tagResult.printToStream(4, outFile);
        }
    }

    // Close the file after writing
    outFile.close();
}

void Result::printToCsvFile(ofstream &outputFile) {
	/* Deliberately shadow the process-wide exploration pointer with this snapshot's cell. */
	MemCell *const cell = cellTech;
	if (!cell) {
		cerr << "Result has no associated memory cell." << endl;
		return;
	}
	cout << "in print output csv loop" << endl;
	outputFile << bank->numRowSubArray << "," << bank->numColumnSubArray << "," << bank->stackedDieCount << "," << bank->numActiveSubArrayPerColumn << "," << bank->numActiveSubArrayPerRow << ",";
	outputFile << bank->numRowMat << "," << bank->numColumnMat << "," << bank->numActiveMatPerColumn << "," << bank->numActiveMatPerRow << ",";
	outputFile << bank->subarray.mat.numRow << "," << bank->subarray.mat.numColumn << ",";
	outputFile << bank->muxSenseAmp << "," << bank->muxOutputLev1 << "," << bank->muxOutputLev2 << ",";
	if (inputParameter->designTarget == cache)
		outputFile << bank->numRowPerSet << ",";
	else
		outputFile << "N/A,";
	switch (localWire->wireType) {
	case local_aggressive:
		outputFile << "Local Aggressive" << ",";
		break;
	case local_conservative:
		outputFile << "Local Conservative" << ",";
		break;
	case semi_aggressive:
		outputFile << "Semi-Global Aggressive" << ",";
		break;
	case semi_conservative:
		outputFile << "Semi-Global Conservative" << ",";
		break;
	case global_aggressive:
		outputFile << "Global Aggressive" << ",";
		break;
	case global_conservative:
		outputFile << "Global Conservative" << ",";
		break;
	default:
		outputFile << "DRAM Wire" << ",";
	}
	switch (localWire->wireRepeaterType) {
	case repeated_none:
		outputFile << "No Repeaters" << ",";
		break;
	case repeated_opt:
		outputFile << "Fully-Optimized Repeaters" << ",";
		break;
	case repeated_5:
		outputFile << "Repeaters with 5% Overhead" << ",";
		break;
	case repeated_10:
		outputFile << "Repeaters with 10% Overhead" << ",";
		break;
	case repeated_20:
		outputFile << "Repeaters with 20% Overhead" << ",";
		break;
	case repeated_30:
		outputFile << "Repeaters with 30% Overhead" << ",";
		break;
	case repeated_40:
		outputFile << "Repeaters with 40% Overhead" << ",";
		break;
	case repeated_50:
		outputFile << "Repeaters with 50% Overhead" << ",";
		break;
	default:
		outputFile << "N/A" << ",";
	}
	if (localWire->isLowSwing)
		outputFile << "Yes" << ",";
	else
		outputFile << "No" << ",";
	switch (globalWire->wireType) {
	case local_aggressive:
		outputFile << "Local Aggressive" << ",";
		break;
	case local_conservative:
		outputFile << "Local Conservative" << ",";
		break;
	case semi_aggressive:
		outputFile << "Semi-Global Aggressive" << ",";
		break;
	case semi_conservative:
		outputFile << "Semi-Global Conservative" << ",";
		break;
	case global_aggressive:
		outputFile << "Global Aggressive" << ",";
		break;
	case global_conservative:
		outputFile << "Global Conservative" << ",";
		break;
	default:
		outputFile << "DRAM Wire" << ",";
	}
	switch (globalWire->wireRepeaterType) {
	case repeated_none:
		outputFile << "No Repeaters" << ",";
		break;
	case repeated_opt:
		outputFile << "Fully-Optimized Repeaters" << ",";
		break;
	case repeated_5:
		outputFile << "Repeaters with 5% Overhead" << ",";
		break;
	case repeated_10:
		outputFile << "Repeaters with 10% Overhead" << ",";
		break;
	case repeated_20:
		outputFile << "Repeaters with 20% Overhead" << ",";
		break;
	case repeated_30:
		outputFile << "Repeaters with 30% Overhead" << ",";
		break;
	case repeated_40:
		outputFile << "Repeaters with 40% Overhead" << ",";
		break;
	case repeated_50:
		outputFile << "Repeaters with 50% Overhead" << ",";
		break;
	default:
		outputFile << "N/A" << ",";
	}
	if (globalWire->isLowSwing)
		outputFile << "Yes" << ",";
	else
		outputFile << "No" << ",";
	switch (bank->areaOptimizationLevel) {
	case latency_first:
		outputFile << "Latency-Optimized" << ",";
		break;
	case area_first:
		outputFile << "Area-Optimized" << ",";
		break;
	default:	/* balance */
		outputFile << "Balanced" << ",";
	}
	outputFile << bank->height * 1e6 << "," << bank->width * 1e6 << "," << bank->area * 1e6 << ",";
	outputFile << bank->subarray.height * 1e6 << "," << bank->subarray.width * 1e6 << "," << bank->subarray.area * 1e6 << ",";
	outputFile << bank->subarray.mat.height * 1e6 << "," << bank->subarray.mat.width * 1e6 << "," << bank->subarray.mat.area * 1e6 << ",";
	outputFile << cell->area * tech->featureSize * tech->featureSize * bank->capacity / bank->area * 100 << ",";
	outputFile << bank->readLatency * 1e9 << "," << bank->writeLatency * 1e9 << ",";
    if (IsRefreshMemory(cell->memCellType)) {
        outputFile << bank->refreshLatency * 1e9 << ",";
    } else {
        outputFile << "0,";
    }
	outputFile << bank->readDynamicEnergy * 1e12 << "," << bank->writeDynamicEnergy * 1e12 << ",";
    if (IsRefreshMemory(cell->memCellType)) {
        outputFile << bank->refreshDynamicEnergy * 1e12 << ",";
    } else {
        outputFile << "0,";
    }
	outputFile << bank->leakage * 1e3 << ",";
    if (IsRefreshMemory(cell->memCellType)) {
        outputFile << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime)) << ",";
    } else {
        outputFile << "0,";
    }

	if (inputParameter->monolithic3DMat) {
        outputFile << bank->subarray.mat.stackedMemTiers << ",";
    } else {
        outputFile << "0,";
    }

	outputFile << "\n";
}

void Result::printAsCacheToCsvFile(Result &tagResult, CacheAccessMode cacheAccessMode, ofstream &outputFile) {
	/* Cache-level refresh behavior follows the saved data-array cell. */
	MemCell *const cell = cellTech;
	if (!cell) {
		cerr << "Result has no associated memory cell." << endl;
		return;
	}
	if (bank->memoryType != MemoryType::data || tagResult.bank->memoryType != MemoryType::tag) {
		cout << "This is not a valid cache configuration." << endl;
		return;
	} else {
		double cacheHitLatency, cacheMissLatency, cacheWriteLatency;
		double cacheHitDynamicEnergy, cacheMissDynamicEnergy, cacheWriteDynamicEnergy;
		double cacheLeakage;
		double cacheArea;
		if (cacheAccessMode == normal_access_mode) {
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;		/* only the tag access latency */
			cacheHitLatency = MAX(tagResult.bank->readLatency, bank->subarray.readLatency);	/* access tag and activate data row in parallel */
			cacheHitLatency += bank->subarray.mat.columnDecoderLatency;		/* add column decoder latency after hit signal arrives */
			cacheHitLatency += bank->readLatency - bank->subarray.readLatency;	/* H-tree in and out latency */
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);	/* Data and tag are written in parallel */
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no subarrayter what tag is always accessed */
			cacheMissDynamicEnergy += bank->readDynamicEnergy;	/* data is also partially accessed, TO-DO: not accurate here */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		} else if (cacheAccessMode == fast_access_mode) {
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;
			cacheHitLatency = MAX(tagResult.bank->readLatency, bank->readLatency);
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no subarrayter what tag is always accessed */
			cacheMissDynamicEnergy += bank->readDynamicEnergy;	/* data is also partially accessed, TO-DO: not accurate here */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		} else {		/* sequential access */
			/* Calculate latencies */
			cacheMissLatency = tagResult.bank->readLatency;
			cacheHitLatency = tagResult.bank->readLatency + bank->readLatency;
			cacheWriteLatency = MAX(tagResult.bank->writeLatency, bank->writeLatency);
			/* Calculate power */
			cacheMissDynamicEnergy = tagResult.bank->readDynamicEnergy;	/* no subarrayter what tag is always accessed */
			cacheHitDynamicEnergy = tagResult.bank->readDynamicEnergy + bank->readDynamicEnergy;
			cacheWriteDynamicEnergy = tagResult.bank->writeDynamicEnergy + bank->writeDynamicEnergy;
		}
		/* Calculate leakage */
		cacheLeakage = tagResult.bank->leakage + bank->leakage;
		/* Calculate area */
		cacheArea = tagResult.bank->area + bank->area;	/* TO-DO: simply add them together here */

		/* start printing */
		switch (cacheAccessMode) {
		case normal_access_mode:
			outputFile << "Normal" << ",";
			break;
		case fast_access_mode:
			outputFile << "Fast" << ",";
			break;
		default:	/* sequential */
			outputFile << "Sequential" << ",";
		}
		outputFile << cacheArea * 1e6 << ",";
		outputFile << cacheHitLatency * 1e9 << ",";
		outputFile << cacheMissLatency * 1e9 << ",";
		outputFile << cacheWriteLatency * 1e9 << ",";
        if (IsRefreshMemory(cell->memCellType)) {
            outputFile << MAX(tagResult.bank->refreshLatency, bank->refreshLatency) * 1e9 << ",";
        } else {
            outputFile << "0,";
        }
		outputFile << cacheHitDynamicEnergy * 1e9 << ",";
		outputFile << cacheMissDynamicEnergy * 1e9 << ",";
		outputFile << cacheWriteDynamicEnergy * 1e9 << ",";
        if (IsRefreshMemory(cell->memCellType)) {
            outputFile << (tagResult.bank->refreshDynamicEnergy + bank->refreshDynamicEnergy) * 1e9 << ",";
        } else {
            outputFile << "0,";
        }
		outputFile << cacheLeakage * 1e3 << ",";
        if (IsRefreshMemory(cell->memCellType)) {
            outputFile << TO_WATT(bank->refreshDynamicEnergy / (cell->retentionTime)) << ",";
        } else {
            outputFile << "0,";
        }
		printToCsvFile(outputFile);
		tagResult.printToCsvFile(outputFile);
        outputFile << bank->subarray.mat.leakage + tagResult.bank->subarray.mat.leakage << ",";
        outputFile << (bank->subarray.mat.area + tagResult.bank->subarray.mat.area) * 1e6 << ",";
		outputFile << endl;
	}
}
