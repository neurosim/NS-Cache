#include <cassert>
#include <cmath>
#include <fstream>
#include <string>

#include "Mat.h"
#include "Result.h"
#include "global.h"

InputParameter *inputParameter = nullptr;
Technology *tech = nullptr;
Technology *devtech = nullptr;
MemCell *cell = nullptr;
Wire *localWire = nullptr;
Wire *globalWire = nullptr;
MemCell **sweepCells = nullptr;

int main(int argc, char **argv) {
	Mat source;
	assert(source.precharger.voltagePrecharge == 0);
	assert(source.precharger.wireLength == 0);
	assert(source.rowDecoder.wireLength == 0);
	assert(source.bitlineMux.capOutput == 0);
	assert(source.senseAmp.pitchSenseAmp == 0);
	assert(source.tsvArray.res == 0);
	assert(source.tsvArray.numReadBits == 0);
	assert(source.tsvArray.C_load_TSV == 0);
	assert(source.tsvArray.F == 0);
	source.dramTiming.accessLatency = 1;
	source.dramTiming.restoreDelay = 2;
	source.dramTiming.readCycleLatency = 3;
	source.dramTiming.writeBitlineDelay = 4;
	source.gcDramPower.readBitlineAccessEnergy = 5;
	source.gcDramPower.writeBitlineAccessEnergy = 6;
	source.gcDramPower.writeChargeDriverEnergy = 7;
	source.gcDramPower.aosLeakageUpperBound = 8;
	source.m3d.mivsPerTier = 9;
	source.m3d.totalMivCount = 10;
	source.m3d.totalMivArea = 11;
	source.m3d.peripheralLogicArea = 12;
	source.m3d.finalLogicLayerArea = 13;
	source.m3d.perTierMemoryArea = 14;
	source.m3d.projectedArea = 15;
	source.m3d.dominantTier = M3DDominantTier::memory;
	source.aosLeakageUpperBound = 16;
	source.aosReadOperatingPoint.Ion = 17;
	source.aosWriteOperatingPoint.Ioff = 18;
	source.stackedMemTiers = 4;
	source.precharger.voltagePrecharge = 19;
	source.precharger.widthInvNmos = 20;
	source.precharger.widthInvPmos = 21;
	source.precharger.wireLength = 22;
	source.precharger.newHeight = 23;
	source.rowDecoder.wireLength = 24;
	source.rowDecoder.refreshLatency = 25;
	source.bitlineMux.refreshDynamicEnergy = 26;
	source.senseAmp.newWidth = 27;
	source.tsvArray.numReadBits = 28;
	source.tsvArray.res = 29;
	source.tsvArray.newHeight = 30;
	source.tsvArray.tsv_type = Monolithic;
	source.tsvArray.invalid = false;

	Mat destination;
	assert(&(destination = source) == &destination);
	assert(destination.dramTiming.accessLatency == 1);
	assert(destination.dramTiming.restoreDelay == 2);
	assert(destination.dramTiming.readCycleLatency == 3);
	assert(destination.dramTiming.writeBitlineDelay == 4);
	assert(destination.gcDramPower.readBitlineAccessEnergy == 5);
	assert(destination.gcDramPower.writeBitlineAccessEnergy == 6);
	assert(destination.gcDramPower.writeChargeDriverEnergy == 7);
	assert(destination.gcDramPower.aosLeakageUpperBound == 8);
	assert(destination.m3d.mivsPerTier == 9);
	assert(destination.m3d.totalMivCount == 10);
	assert(destination.m3d.totalMivArea == 11);
	assert(destination.m3d.peripheralLogicArea == 12);
	assert(destination.m3d.finalLogicLayerArea == 13);
	assert(destination.m3d.perTierMemoryArea == 14);
	assert(destination.m3d.projectedArea == 15);
	assert(destination.m3d.dominantTier == M3DDominantTier::memory);
	assert(destination.aosLeakageUpperBound == 16);
	assert(destination.aosReadOperatingPoint.Ion == 17);
	assert(destination.aosWriteOperatingPoint.Ioff == 18);
	assert(destination.stackedMemTiers == 4);
	assert(destination.precharger.voltagePrecharge == 19);
	assert(destination.precharger.widthInvNmos == 20);
	assert(destination.precharger.widthInvPmos == 21);
	assert(destination.precharger.wireLength == 22);
	assert(destination.precharger.newHeight == 23);
	assert(destination.rowDecoder.wireLength == 24);
	assert(destination.rowDecoder.refreshLatency == 25);
	assert(destination.bitlineMux.refreshDynamicEnergy == 26);
	assert(destination.senseAmp.newWidth == 27);
	assert(destination.tsvArray.numReadBits == 28);
	assert(destination.tsvArray.res == 29);
	assert(destination.tsvArray.newHeight == 30);
	assert(destination.tsvArray.tsv_type == Monolithic);
	assert(!destination.tsvArray.invalid);

	Technology localTechnology;
	localTechnology.featureSize = 10e-9;
	localTechnology.pnSizeRatio = 2;
	localTechnology.currentOnNmos[0] = 1;
	localTechnology.capTSV[Fine] = 1e-15;
	localTechnology.resTSV[Fine] = 2;
	localTechnology.areaTSV[Fine] = 3;
	tech = &localTechnology;
	TSV initializedTsv;
	initializedTsv.Initialize(Fine, false);
	assert(initializedTsv.initialized);
	assert(!initializedTsv.invalid);
	assert(initializedTsv.tsv_type == Fine);
	assert(initializedTsv.cap == 1e-15);
	assert(initializedTsv.res == 2);
	assert(initializedTsv.min_area == 3e-12);
	assert(initializedTsv.C_load_TSV == 0);
	assert(initializedTsv.F == 0);

	InputParameter localInputParameter;
	localInputParameter.routingMode = h_tree;
	localInputParameter.temperature = 300;
	localInputParameter.maxNmosSize = 100;
	inputParameter = &localInputParameter;
	OutputDriver zeroStageDriver;
	zeroStageDriver.Initialize(1, 1e-15, 1e-15, 0, false, latency_first, 0, false, 0);
	assert(zeroStageDriver.initialized);
	assert(!zeroStageDriver.invalid);
	assert(zeroStageDriver.numStage == 0);
	zeroStageDriver.CalculateArea();
	zeroStageDriver.CalculateRC();
	zeroStageDriver.CalculateLatency(7e-12);
	zeroStageDriver.CalculatePower();
	assert(zeroStageDriver.area == 0);
	assert(zeroStageDriver.capInput[0] == 0);
	assert(zeroStageDriver.readLatency == 0);
	assert(zeroStageDriver.writeLatency == 0);
	assert(zeroStageDriver.rampOutput == 7e-12);
	assert(zeroStageDriver.readDynamicEnergy == 0);
	assert(zeroStageDriver.writeDynamicEnergy == 0);
	assert(zeroStageDriver.leakage == 0);
	MemCell globalCell;
	globalCell.memCellType = SRAM;
	MemCell ownedCell;
	ownedCell.memCellType = eDRAM;
	cell = &globalCell;
	Result result;
	assert(result.cellTech == &globalCell);
	result.cellTech = &ownedCell;
	result.bank->readLatency = 10;
	result.bank->writeLatency = 10;
	result.bank->blockSize = 64;
	result.bank->subarray.mat.readLatency = 8;
	result.bank->subarray.mat.rowDecoder.readLatency = 2;
	result.bank->subarray.mat.precharger.readLatency = 1;
	result.bank->subarray.mat.dramTiming.readCycleLatency = 4;
	result.bank->subarray.mat.writeLatency = 5;
	assert(std::abs(result.getReadBandwidth() - 2) < 1e-12);
	assert(std::abs(result.getWriteBandwidth() - 1.6) < 1e-12);
	globalCell.memCellType = gcDRAM;
	assert(std::abs(result.getReadBandwidth() - 2) < 1e-12);
	assert(std::abs(result.getWriteBandwidth() - 1.6) < 1e-12);

	if (argc == 2) {
		localInputParameter.designTarget = cache;
		localInputParameter.internalSensing = true;
		localInputParameter.monolithic3DMat = false;
		localInputParameter.printLevel = 1;
		localInputParameter.quantize = false;
		localInputParameter.viewMatStats = false;
		ownedCell.area = 1;
		ownedCell.retentionTime = 1;
		globalCell.memCellType = SRAM;
		globalCell.area = 1;

		auto prepareBank = [](Bank *bank, MemoryType memoryType) {
			bank->memoryType = memoryType;
			bank->internalSenseAmp = true;
			bank->numRowSubArray = bank->numColumnSubArray = 1;
			bank->numActiveSubArrayPerRow = bank->numActiveSubArrayPerColumn = 1;
			bank->numRowMat = bank->numColumnMat = 1;
			bank->numActiveMatPerRow = bank->numActiveMatPerColumn = 1;
			bank->muxSenseAmp = bank->muxOutputLev1 = bank->muxOutputLev2 = 1;
			bank->numRowPerSet = 1;
			bank->stackedDieCount = 1;
			bank->partitionGranularity = 0;
			bank->capacity = 1024;
			bank->blockSize = 64;
			bank->height = bank->width = 1e-3;
			bank->area = 1e-6;
			bank->readLatency = bank->writeLatency = 1e-9;
			bank->readDynamicEnergy = bank->writeDynamicEnergy = 1e-12;
			bank->refreshLatency = 1e-6;
			bank->refreshDynamicEnergy = 1e-10;
			bank->leakage = 1e-3;
			bank->routingReadLatency = bank->routingWriteLatency = 0;
			bank->routingResetLatency = bank->routingSetLatency = 0;
			bank->routingRefreshLatency = 0;
			bank->routingReadDynamicEnergy = bank->routingWriteDynamicEnergy = 0;
			bank->routingResetDynamicEnergy = bank->routingSetDynamicEnergy = 0;
			bank->routingRefreshDynamicEnergy = bank->routingLeakage = 0;

			bank->subarray.memoryType = memoryType;
			bank->subarray.internalSenseAmp = true;
			bank->subarray.numRowMat = bank->subarray.numColumnMat = 1;
			bank->subarray.numActiveMatPerRow = bank->subarray.numActiveMatPerColumn = 1;
			bank->subarray.height = bank->subarray.width = 1e-3;
			bank->subarray.area = 1e-6;
			bank->subarray.areaAllLogicBlocks = 1e-7;
			bank->subarray.readLatency = bank->subarray.writeLatency = 1e-9;
			bank->subarray.refreshLatency = 1e-6;
			bank->subarray.readDynamicEnergy = bank->subarray.writeDynamicEnergy = 1e-12;
			bank->subarray.refreshDynamicEnergy = 1e-10;
			bank->subarray.predecoderLatency = 0;

			bank->subarray.mat.numRow = bank->subarray.mat.numColumn = 1;
			bank->subarray.mat.muxSenseAmp = 1;
			bank->subarray.mat.muxOutputLev1 = bank->subarray.mat.muxOutputLev2 = 1;
			bank->subarray.mat.height = bank->subarray.mat.width = 1e-3;
			bank->subarray.mat.area = 1e-6;
			bank->subarray.mat.readLatency = bank->subarray.mat.writeLatency = 1e-9;
			bank->subarray.mat.refreshLatency = 1e-6;
			bank->subarray.mat.readDynamicEnergy = bank->subarray.mat.writeDynamicEnergy = 1e-12;
			bank->subarray.mat.refreshDynamicEnergy = 1e-10;
		};

		prepareBank(result.bank, MemoryType::data);
		Result tagResult;
		tagResult.cellTech = &globalCell;
		prepareBank(tagResult.bank, MemoryType::tag);
		for (Result *current : {&result, &tagResult}) {
			current->localWire->wireType = local_aggressive;
			current->localWire->wireRepeaterType = repeated_none;
			current->localWire->isLowSwing = false;
			current->globalWire->wireType = global_aggressive;
			current->globalWire->wireRepeaterType = repeated_none;
			current->globalWire->isLowSwing = false;
		}

		result.printAsCacheToFile(tagResult, normal_access_mode, argv[1]);
		std::ifstream report(argv[1]);
		const std::string contents((std::istreambuf_iterator<char>(report)),
				std::istreambuf_iterator<char>());
		const auto summary = contents.find("CACHE DESIGN -- SUMMARY");
		const auto dataDetails = contents.find("CACHE DATA ARRAY DETAILS");
		const auto tagDetails = contents.find("CACHE TAG ARRAY DETAILS");
		assert(summary != std::string::npos);
		assert(dataDetails != std::string::npos);
		assert(tagDetails != std::string::npos);
		assert(summary < dataDetails && dataDetails < tagDetails);
		assert(contents.find("Tag Array Area") != std::string::npos);
	}
}
