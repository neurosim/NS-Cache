#include <cassert>
#include <cmath>
#include <string>

#include "Mat.h"
#include "SubArray.h"
#include "global.h"

InputParameter *inputParameter = nullptr;
Technology *tech = nullptr;
Technology *devtech = nullptr;
MemCell *cell = nullptr;
Wire *localWire = nullptr;
Wire *globalWire = nullptr;
MemCell **sweepCells = nullptr;

namespace {

bool NearlyEqual(double actual, double expected, double relativeTolerance = 1e-10) {
	return std::fabs(actual - expected)
			<= std::max(std::fabs(actual), std::fabs(expected)) * relativeTolerance;
}

void InitializeMat(const std::string &configPath, Mat *mat, InputParameter *parameters,
		Technology *peripheralTechnology, Technology *cellTechnology,
		MemCell *memoryCell, Wire *local, Wire *global) {
	inputParameter = parameters;
	parameters->ReadInputParameterFromFile(configPath);

	tech = peripheralTechnology;
	tech->Initialize(parameters->processNode, parameters->deviceRoadmap, parameters);
	tech->SetLayerCount(parameters, 1);

	cell = memoryCell;
	cell->ReadCellFromFile(parameters->fileMemCell.at(0));
	cell->ApplyPVT();
	devtech = tech;
	if (cell->memCellType == eDRAM) {
		cellTechnology->Initialize(parameters->processNode, EDRAM, parameters);
		cellTechnology->SetLayerCount(parameters, 1);
		devtech = cellTechnology;
	}

	localWire = local;
	globalWire = global;
	local->Initialize(parameters->processNode,
			static_cast<WireType>(parameters->minLocalWireType),
			static_cast<WireRepeaterType>(parameters->minLocalWireRepeaterType),
			parameters->temperature, parameters->minIsLocalWireLowSwing);
	global->Initialize(parameters->processNode,
			static_cast<WireType>(parameters->minGlobalWireType),
			static_cast<WireRepeaterType>(parameters->minGlobalWireRepeaterType),
			parameters->temperature, parameters->minIsGlobalWireLowSwing);

	mat->Initialize(512, 128, false, true, 1, true, 1, 1, latency_first, 1);
	assert(!mat->invalid);
	mat->CalculateArea();
	assert(!mat->invalid);
}

void CheckEDRAMSettling(const std::string &configPath) {
	InputParameter parameters;
	Technology peripheralTechnology;
	Technology cellTechnology;
	MemCell memoryCell;
	Wire local;
	Wire global;
	Mat mat;
	InitializeMat(configPath, &mat, &parameters, &peripheralTechnology,
			&cellTechnology, &memoryCell, &local, &global);
	mat.CalculateLatency(infinite_ramp);
	assert(!mat.invalid);

	const double pathResistance = mat.resBitline + mat.resCellAccess;
	const double pathCapacitance = cell->capDRAMCell + mat.capCellAccess + mat.capBitline
			+ mat.bitlineMux.capForPreviousDelayCalculation;
	const double residual = parameters.dramTargetResidualRatio;
	const double expectedWrite = pathResistance * pathCapacitance * std::log(1.0 / residual);
	assert(NearlyEqual(mat.writeBitlineDelay, expectedWrite));
	assert(NearlyEqual(std::exp(-mat.writeBitlineDelay / (pathResistance * pathCapacitance)),
			residual));

	const double fullSwing = std::fabs(cell->resetVoltage - mat.voltagePrecharge);
	const double remainingSwing = std::max(fullSwing - mat.senseVoltage, 0.0);
	const double targetResidual = fullSwing * residual;
	const double expectedRestore = remainingSwing > targetResidual
			? pathResistance * pathCapacitance * std::log(remainingSwing / targetResidual) : 0;
	assert(NearlyEqual(mat.dramTiming.restoreDelay, expectedRestore));
	assert(NearlyEqual(mat.dramTiming.readCycleLatency,
			mat.dramTiming.accessLatency + mat.dramTiming.restoreDelay));
	assert(NearlyEqual(mat.refreshLatency,
			mat.dramTiming.readCycleLatency * static_cast<double>(mat.numRow)));
}

void CheckGcDRAMSettlingAndRefresh(const std::string &configPath) {
	InputParameter parameters;
	Technology peripheralTechnology;
	Technology unusedCellTechnology;
	MemCell memoryCell;
	Wire local;
	Wire global;
	Mat mat;
	InitializeMat(configPath, &mat, &parameters, &peripheralTechnology,
			&unusedCellTechnology, &memoryCell, &local, &global);
	assert(cell->memCellType == gcDRAM);
	mat.CalculateLatency(infinite_ramp);
	assert(!mat.invalid);

	const double pathResistance = mat.resBitline + mat.resWriteCellAccess;
	const double pathCapacitance = mat.capWriteCellAccess + mat.capReadCellGate
			+ mat.capBitline + mat.bitlineMux.capForPreviousDelayCalculation;
	const double residual = parameters.dramTargetResidualRatio;
	const double expectedWrite = pathResistance * pathCapacitance * std::log(1.0 / residual);
	assert(NearlyEqual(mat.writeBitlineDelay, expectedWrite));

	mat.CalculatePower();
	assert(!mat.invalid);
	const double expectedRefreshPerRow = mat.gcDramPower.readBitlineAccessEnergy
			+ mat.gcDramPower.writeBitlineAccessEnergy
			+ mat.rowDecoder.readDynamicEnergy + mat.gcRowDecoder.readDynamicEnergy
			+ mat.precharger.readDynamicEnergy + mat.writecharger.readDynamicEnergy
			+ mat.senseAmp.readDynamicEnergy;
	assert(NearlyEqual(mat.refreshDynamicEnergy,
			expectedRefreshPerRow * static_cast<double>(mat.numRow + 2)));
}

void CheckM3DLoadPropagation(const std::string &configPath, bool gainCell) {
	InputParameter parameters;
	Technology peripheralTechnology;
	Technology cellTechnology;
	MemCell memoryCell;
	Wire local;
	Wire global;
	Mat mat;
	InitializeMat(configPath, &mat, &parameters, &peripheralTechnology,
			&cellTechnology, &memoryCell, &local, &global);
	assert(parameters.monolithic3DMat);
	assert(mat.stackedMemTiers >= 1);
	assert(mat.tsvArray.width > 0);
	assert(NearlyEqual(mat.tsvArray.width, std::sqrt(mat.tsvArray.area)));
	assert(NearlyEqual(mat.tsvArray.height, mat.tsvArray.width));
	assert(NearlyEqual(mat.rowDecoder.capLoad,
			mat.sectioncap + (mat.numRepeaters ? mat.gateCapRep : 0)));
	assert(NearlyEqual(mat.rowDecoder.outputDriver.outputCap, mat.rowDecoder.capLoad));
	assert(NearlyEqual(mat.rowDecoder.outputDriver.outputRes, mat.rowDecoder.resLoad));
	const double expectedMuxCap = mat.sectioncapMux
			+ (mat.numRepeaters ? mat.gateCapRep : 0);
	assert(NearlyEqual(mat.bitlineMuxDecoder.outputDriver.outputCap, expectedMuxCap));
	assert(NearlyEqual(mat.senseAmpMuxLev1Decoder.outputDriver.outputCap, expectedMuxCap));
	assert(NearlyEqual(mat.senseAmpMuxLev2Decoder.outputDriver.outputCap, expectedMuxCap));
	assert(NearlyEqual(mat.bitlineMuxDecoder.outputDriver.outputRes, mat.sectionresMux));
	assert(NearlyEqual(mat.senseAmpMuxLev1Decoder.outputDriver.outputRes, mat.sectionresMux));
	assert(NearlyEqual(mat.senseAmpMuxLev2Decoder.outputDriver.outputRes, mat.sectionresMux));
	assert(NearlyEqual(mat.precharger.resBitline, mat.resBitline));
	if (gainCell) {
		assert(cell->memCellType == gcDRAM);
		assert(NearlyEqual(mat.precharger.capBitline, mat.capBitlineRead));
		assert(NearlyEqual(mat.writecharger.capBitline, mat.capBitline));
		assert(NearlyEqual(mat.writecharger.resBitline, mat.resBitline));
		assert(NearlyEqual(mat.gcRowDecoder.outputDriver.outputCap, mat.capWordlineRead));
		assert(NearlyEqual(mat.gcRowDecoder.outputDriver.outputRes, mat.resWordline));
	} else {
		assert(NearlyEqual(mat.precharger.capBitline, mat.capBitline));
	}
}

void CheckForcedMatDoesNotFilterTags(const std::string &configPath) {
	InputParameter parameters;
	Technology peripheralTechnology;
	Technology cellTechnology;
	MemCell memoryCell;
	Wire local;
	Wire global;
	Mat initializedDataMat;
	InitializeMat(configPath, &initializedDataMat, &parameters, &peripheralTechnology,
			&cellTechnology, &memoryCell, &local, &global);
	assert(parameters.forceMatSize);

	SubArray tagArray;
	tagArray.Initialize(1, 1, 6, 32, 1, 1, false, 1, 1, 1, true, 1, 1,
			latency_first, MemoryType::tag, 1, 0, 1);
	assert(!tagArray.invalid);
	assert(static_cast<uint64_t>(tagArray.mat.numRow) != parameters.forcedMatRows
			|| static_cast<uint64_t>(tagArray.mat.numColumn) != parameters.forcedMatColumns);
}

}  // namespace

int main(int argc, char **argv) {
	assert(argc == 5);
	CheckEDRAMSettling(argv[1]);
	CheckGcDRAMSettlingAndRefresh(argv[2]);
	CheckM3DLoadPropagation(argv[3], false);
	CheckM3DLoadPropagation(argv[4], true);
	CheckForcedMatDoesNotFilterTags(argv[3]);
}
