#include "global.h"

#include <iostream>

InputParameter *inputParameter = NULL;
Technology *tech = NULL;
Technology *devtech = NULL;
MemCell *cell = NULL;
Technology *gtech = NULL;
Wire *localWire = NULL;
Wire *globalWire = NULL;
MemCell **sweepCells = NULL;

int main(int argc, char **argv) {
	if (argc != 2)
		return 2;
	InputParameter parameters;
	Technology technology;
	parameters.temperature = 300;
	technology.vdd = 1.0;
	inputParameter = &parameters;
	tech = &technology;
	MemCell parsedCell;
	cell = &parsedCell;
	parsedCell.ReadCellFromFile(argv[1]);
	if (!parsedCell.oxideTransistor) {
		std::cout << "cmos\n";
		return 0;
	}
	if (parsedCell.memCellType == eDRAM) {
		AOSDeviceOperatingPoint point =
				parsedCell.oxideAccessTransistor.operatingPoint;
		double suppliedRetention = parsedCell.retentionTime;
		parsedCell.ApplyPVT();
		std::cout << "edram " << point.Ion << " " << point.Ioff
				<< " retention-ratio=" << parsedCell.retentionTime / suppliedRetention << "\n";
	} else if (parsedCell.memCellType == gcDRAM) {
		AOSDeviceOperatingPoint readPoint =
				parsedCell.oxideReadTransistor.operatingPoint;
		AOSDeviceOperatingPoint writePoint =
				parsedCell.oxideWriteTransistor.operatingPoint;
		std::cout << "gcdram " << readPoint.Ion << " " << writePoint.Ion << "\n";
	} else {
		return 4;
	}
	return 0;
}
