#include <iomanip>
#include <iostream>
#include <string>

#include "InputParameter.h"

class MemCell;
MemCell *cell = NULL;

int main(int argc, char **argv) {
	if (argc < 2 || argc > 3) {
		std::cerr << "usage: config_probe <config> [--print]" << std::endl;
		return 2;
	}

	InputParameter parameters;
	parameters.ReadInputParameterFromFile(argv[1]);
	std::cout << std::setprecision(17)
			<< "relax=" << parameters.relaxSRAMCell << '\n'
			<< "aspect=" << parameters.bankAspectRatioLimit << '\n'
			<< "force=" << parameters.forceMatSize << '\n'
			<< "force_rows=" << parameters.forcedMatRows << '\n'
			<< "force_columns=" << parameters.forcedMatColumns << '\n'
			<< "dram_residual=" << parameters.dramTargetResidualRatio << '\n'
			<< "m3d=" << parameters.monolithic3DMat << '\n'
			<< "max_tiers=" << parameters.maxMatLayers << std::endl;

	if (argc == 3) {
		if (std::string(argv[2]) != "--print")
			return 2;
		parameters.PrintInputParameter();
	}
	return 0;
}
