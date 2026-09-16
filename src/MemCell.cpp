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


#include "MemCell.h"
#include "formula.h"
#include "global.h"
#include "macros.h"
#include <cerrno>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <math.h>
#include <stdexcept>
#include <string>

namespace {

bool IsAOSCell(MemCellType type) {
	return type == eDRAM || type == gcDRAM;
}

string Trim(const string &value) {
	size_t first = 0;
	while (first < value.size() && isspace(static_cast<unsigned char>(value[first])))
		++first;
	size_t last = value.size();
	while (last > first && isspace(static_cast<unsigned char>(value[last - 1])))
		--last;
	return value.substr(first, last - first);
}

string Lower(string value) {
	for (size_t i = 0; i < value.size(); ++i)
		value[i] = static_cast<char>(tolower(static_cast<unsigned char>(value[i])));
	return value;
}

void FailCellFile(const string &inputFile, const string &message) {
	cerr << "Invalid cell file '" << inputFile << "': " << message << endl;
	exit(-1);
}

bool SplitSetting(const string &line, string *key, string *value) {
	size_t colon = line.find(':');
	if (colon == string::npos)
		return false;
	*key = Trim(line.substr(0, colon));
	*value = Trim(line.substr(colon + 1));
	return true;
}

double ParseFiniteCellDouble(const string &inputFile, const string &key,
		const string &value) {
	if (value.empty())
		FailCellFile(inputFile, key + " requires a value");
	errno = 0;
	char *end = NULL;
	double parsed = strtod(value.c_str(), &end);
	while (end != NULL && isspace(static_cast<unsigned char>(*end)))
		++end;
	if (end == value.c_str() || end == NULL || *end != '\0' || errno == ERANGE
			|| !std::isfinite(parsed)) {
		FailCellFile(inputFile, key + " requires one finite numeric value with no trailing text");
	}
	return parsed;
}

double ParseFiniteCellVoltage(const string &inputFile, const string &key,
		const string &value) {
	if (Lower(value) == "vdd") {
		if (tech == NULL || !std::isfinite(tech->vdd))
			FailCellFile(inputFile, key + " cannot resolve vdd before technology initialization");
		return tech->vdd;
	}
	return ParseFiniteCellDouble(inputFile, key, value);
}

bool ParseStrictCellBool(const string &inputFile, const string &key,
		const string &value) {
	string normalized = Lower(value);
	if (normalized == "true" || normalized == "yes" || normalized == "1")
		return true;
	if (normalized == "false" || normalized == "no" || normalized == "0")
		return false;
	FailCellFile(inputFile, key + " expects yes/no, true/false, or 1/0");
	return false;
}

void InitializeAOSParameterSet(OxideTransistorParameterSet *transistor) {
	transistor->initialized = false;
	transistor->temperatureSpecified = false;
	transistor->widthSpecified = false;
	transistor->lengthSpecified = false;
	transistor->overlapCapacitanceSpecified = false;
	transistor->mobilitySpecified = false;
	transistor->mobilityScaleSpecified = false;
	transistor->leakageScaleSpecified = false;
	transistor->flatBandVoltageSpecified = false;
	transistor->contactResistanceSpecified = false;
	transistor->wordlineBoostVoltageSpecified = false;
	transistor->wordlineHoldVoltageSpecified = false;
	transistor->wordlineBoostVoltage = 0;
	transistor->wordlineHoldVoltage = 0;
	transistor->parameters = AOSFETCompactModel::DefaultParameters();
	transistor->operatingPoint = AOSDeviceOperatingPoint{};
}

bool ParseAOSParameter(const string &inputFile, const string &rawKey,
		const string &value, const string &prefix,
		OxideTransistorParameterSet *transistor) {
	if (rawKey.compare(0, prefix.size(), prefix) != 0)
		return false;
	/* AOS dimensions and calibration values are unit-sensitive.  Match the
	 * complete legacy key so a misspelled or different unit cannot silently be
	 * interpreted as the unit expected by the compact model. */
	string field = rawKey.substr(prefix.size());
	if (field.empty())
		FailCellFile(inputFile, "missing AOS field name after " + prefix);
	double parsed = 0;
	bool handled = true;

	if (field == "Temperature (K)") {
		parsed = ParseFiniteCellDouble(inputFile, rawKey, value);
		transistor->parameters.temperature = parsed;
		transistor->temperatureSpecified = true;
	} else if (field == "TailTemperature (K)") {
		transistor->parameters.tailTemperature = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "Width (m)") {
		transistor->parameters.width = ParseFiniteCellDouble(inputFile, rawKey, value);
		transistor->widthSpecified = true;
	} else if (field == "Length (m)") {
		transistor->parameters.length = ParseFiniteCellDouble(inputFile, rawKey, value);
		transistor->lengthSpecified = true;
	} else if (field == "OverlapCapacitance (F)") {
		transistor->parameters.overlapCapacitance = ParseFiniteCellDouble(inputFile, rawKey, value);
		transistor->overlapCapacitanceSpecified = true;
	} else if (field == "FlatBandVoltage (V)") {
		transistor->parameters.flatBandVoltage = ParseFiniteCellDouble(inputFile, rawKey, value);
		transistor->flatBandVoltageSpecified = true;
	} else if (field == "Mobility (cm^2/Vs)" || field == "Mu (cm^2/Vs)") {
		transistor->parameters.mobility = ParseFiniteCellDouble(inputFile, rawKey, value) * 1e-4;
		transistor->parameters.useConstantMobility = true;
		transistor->mobilitySpecified = true;
	} else if (field == "MobilityScale") {
		transistor->parameters.mobilityScale = ParseFiniteCellDouble(inputFile, rawKey, value);
		transistor->mobilityScaleSpecified = true;
	} else if (field == "LeakageScale") {
		transistor->parameters.leakageScale = ParseFiniteCellDouble(inputFile, rawKey, value);
		transistor->leakageScaleSpecified = true;
	} else if (field == "TotalContactResistance (kOhm-um)"
			|| field == "TotalContactResistanceKohmUm") {
		transistor->parameters.totalContactResistanceKohmUm =
				ParseFiniteCellDouble(inputFile, rawKey, value);
		transistor->contactResistanceSpecified = true;
	} else if (field == "BoostVoltage (V)" || field == "OnGateVoltage (V)") {
		transistor->wordlineBoostVoltage = ParseFiniteCellVoltage(inputFile, rawKey, value);
		transistor->wordlineBoostVoltageSpecified = true;
	} else if (field == "HoldVoltage (V)" || field == "OffGateVoltage (V)") {
		transistor->wordlineHoldVoltage = ParseFiniteCellVoltage(inputFile, rawKey, value);
		transistor->wordlineHoldVoltageSpecified = true;
	} else if (field == "ElementaryCharge (C)") {
		transistor->parameters.q = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "BoltzmannConstant (J/K)") {
		transistor->parameters.k = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "VacuumPermittivity (F/m)") {
		transistor->parameters.eps0 = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "SemiconductorPermittivity (F/m)") {
		transistor->parameters.semiconductorPermittivity = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "Nu0T0") {
		transistor->parameters.nu0T0 = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "TrapDensity") {
		transistor->parameters.trapDensity = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "FermiReference") {
		transistor->parameters.fermiReference = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "OxideCapacitance (F/m^2)") {
		transistor->parameters.oxideCapacitance = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "VRHThreshold (V)") {
		transistor->parameters.vrhThreshold = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "PercolationThreshold (V)") {
		transistor->parameters.percolationThreshold = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "VRHPrefactor (cm^2/Vs)") {
		transistor->parameters.vrhPrefactor = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "PercolationPrefactor (cm^2/Vs)") {
		transistor->parameters.percolationPrefactor = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "PercolationExponent") {
		transistor->parameters.percolationExponent = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "LeakageSwingFactor") {
		transistor->parameters.leakageSwingFactor = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "LeakageSurfacePotentialReference (V)") {
		transistor->parameters.leakageSurfacePotentialReference = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else if (field == "CurrentFloor (A)") {
		transistor->parameters.currentFloor = ParseFiniteCellDouble(inputFile, rawKey, value);
	} else {
		handled = false;
	}

	if (!handled)
		FailCellFile(inputFile, "unknown or unused AOS field '" + rawKey + "'");
	transistor->initialized = true;
	return true;
}

void ValidateAOSParameterSet(const string &inputFile, const char *name,
		OxideTransistorParameterSet &transistor) {
	string missing;
	auto AddMissing = [&](const char *field) {
		if (!missing.empty())
			missing += ", ";
		missing += field;
	};
	if (!transistor.temperatureSpecified) AddMissing("temperature");
	if (!transistor.widthSpecified) AddMissing("width");
	if (!transistor.lengthSpecified) AddMissing("length");
	if (!transistor.overlapCapacitanceSpecified) AddMissing("overlap capacitance");
	if (!transistor.flatBandVoltageSpecified) AddMissing("flat-band voltage");
	if (!transistor.leakageScaleSpecified) AddMissing("leakage scale");
	if (!transistor.contactResistanceSpecified) AddMissing("contact resistance");
	if (!transistor.wordlineBoostVoltageSpecified) AddMissing("on/boost gate voltage");
	if (!transistor.wordlineHoldVoltageSpecified) AddMissing("off/hold gate voltage");
	if (!transistor.mobilitySpecified && !transistor.mobilityScaleSpecified)
		AddMissing("Mobility/Mu or MobilityScale");
	if (!missing.empty())
		FailCellFile(inputFile, string(name) + " AOS transistor is incomplete; missing " + missing);
	if (transistor.wordlineBoostVoltage <= transistor.wordlineHoldVoltage)
		FailCellFile(inputFile, string(name) + " AOS on/boost voltage must exceed off/hold voltage");
	if (inputParameter == NULL || tech == NULL || !std::isfinite(tech->vdd) || tech->vdd <= 0)
		FailCellFile(inputFile, string(name) + " AOS validation requires top-level configuration");
	if (fabs(transistor.parameters.temperature - inputParameter->temperature) > 1e-9)
		FailCellFile(inputFile, string(name) + " AOS temperature must match the top-level operating temperature");
	try {
		AOSFETCompactModel model(transistor.parameters);
		transistor.operatingPoint = model.EvaluateOperatingPoint(
				transistor.wordlineBoostVoltage,
				transistor.wordlineHoldVoltage, tech->vdd);
	} catch (const std::exception &error) {
		FailCellFile(inputFile, string(name) + " AOS transistor is invalid: " + error.what());
	}
}

void PrintAOSParameterSet(int indent, const char *name,
		const OxideTransistorParameterSet &transistor) {
	ios::fmtflags flags = cout.flags();
	streamsize precision = cout.precision();
	cout << scientific << setprecision(3);
	cout << string(indent, ' ') << name << " AOS transistor (compact-model v1):" << endl;
	cout << string(indent + 2, ' ') << "Temperature: " << transistor.parameters.temperature << "K" << endl;
	cout << string(indent + 2, ' ') << "Width/length: " << transistor.parameters.width
			<< "m/" << transistor.parameters.length << "m" << endl;
	cout << string(indent + 2, ' ') << "Off/on gate voltage: "
			<< transistor.wordlineHoldVoltage << "V/" << transistor.wordlineBoostVoltage << "V" << endl;
	cout << string(indent + 2, ' ') << "Overlap capacitance: "
			<< transistor.parameters.overlapCapacitance << "F" << endl;
	cout << string(indent + 2, ' ') << "Mobility scale / leakage scale: "
			<< transistor.parameters.mobilityScale << "/" << transistor.parameters.leakageScale << endl;
	cout << string(indent + 2, ' ') << "Total contact resistance: "
			<< transistor.parameters.totalContactResistanceKohmUm << "kOhm-um" << endl;
	cout.flags(flags);
	cout.precision(precision);
}

} // namespace

MemCell::MemCell() {
	// TODO Auto-generated constructor stub
	memCellType         = PCRAM;
	area                = 0;
	aspectRatio         = 0;
	resistanceOn        = 0;
	resistanceOff       = 0;
	readMode            = true;
	readVoltage         = 0;
	readCurrent         = 0;
	readPower           = 0;
        wordlineBoostRatio  = 1.0;
	resetMode           = true;
	resetVoltage        = 0;
	resetCurrent        = 0;
	minSenseVoltage     = 0.08;
	resetPulse          = 0;
	resetEnergy         = 0;
	setMode             = true;
	setVoltage          = 0;
	setCurrent          = 0;
	setPulse            = 0;
	accessType          = CMOS_access;
	processNode         = 0;
	setEnergy           = 0;

	/* Optional */
	stitching         = 0;
	gateOxThicknessFactor = 2;
	widthSOIDevice = 0;
	widthAccessCMOS   = 0;
	voltageDropAccessDevice = 0;
	leakageCurrentAccessDevice = 0;
	capDRAMCell		  = 0;
	widthSRAMCellNMOS = 2.08;	/* Default NMOS width in SRAM cells is 2.08 (from CACTI) */
	widthSRAMCellPMOS = 1.23;	/* Default PMOS width in SRAM cells is 1.23 (from CACTI) */
	oxideTransistor = false;
	InitializeAOSParameterSet(&oxideAccessTransistor);
	InitializeAOSParameterSet(&oxideReadTransistor);
	InitializeAOSParameterSet(&oxideWriteTransistor);

	/*For memristors */
	readFloating = false;
	resistanceOnAtSetVoltage = 0;
	resistanceOffAtSetVoltage = 0;
	resistanceOnAtResetVoltage = 0;
	resistanceOffAtResetVoltage = 0;
	resistanceOnAtReadVoltage = 0;
	resistanceOffAtReadVoltage = 0;
	resistanceOnAtHalfReadVoltage = 0;
	resistanceOffAtHalfReadVoltage = 0;

    retentionTime = invalid_value;
	temperature = 300;
	retentionAOSOffCurrentRatio = false;
	retentionReferenceHoldVoltageSpecified = false;
	retentionReferenceHoldVoltage = 0;
}

MemCell::~MemCell() {
	// TODO Auto-generated destructor stub
}

void MemCell::ReadCellFromFile(const string & inputFile)
{
	FILE *fp = fopen(inputFile.c_str(), "r");
	char line[5000];
	char tmp[5000];
	bool oxideControlSpecified = false;
	bool oxideFieldSpecified = false;
	bool retentionModelSpecified = false;

	if (!fp) {
		cout << inputFile << " cannot be found!\n";
		cout<<" This file may be present in \"config\" folder. If so, please run destiny from that folder, otherwise, change the file name to include folder location.\n";
		exit(-1);
	}

	while (fscanf(fp, "%[^\n]\n", line) != EOF) {
		string trimmedLine = Trim(line);
		string settingKey;
		string settingValue;
		bool hasSetting = SplitSetting(line, &settingKey, &settingValue);
		if (!hasSetting && (trimmedLine.compare(0, strlen("-Oxide"), "-Oxide") == 0
				|| trimmedLine.compare(0, strlen("-RetentionModel"), "-RetentionModel") == 0
				|| trimmedLine.compare(0, strlen("-RetentionReferenceHoldVoltage"),
						"-RetentionReferenceHoldVoltage") == 0)) {
			FailCellFile(inputFile, "malformed AOS/retention setting '" + trimmedLine + "'");
		}
		if (hasSetting) {
			if (settingKey == "-OxideTransistor") {
				if (oxideControlSpecified)
					FailCellFile(inputFile, "-OxideTransistor may be specified only once");
				oxideTransistor = ParseStrictCellBool(inputFile, settingKey, settingValue);
				oxideControlSpecified = true;
				continue;
			}
			if (settingKey.compare(0, strlen("-OxideAccessTransistor"),
					"-OxideAccessTransistor") == 0) {
				ParseAOSParameter(inputFile, settingKey, settingValue,
						"-OxideAccessTransistor", &oxideAccessTransistor);
				oxideFieldSpecified = true;
				continue;
			}
			if (settingKey.compare(0, strlen("-OxideReadTransistor"),
					"-OxideReadTransistor") == 0) {
				ParseAOSParameter(inputFile, settingKey, settingValue,
						"-OxideReadTransistor", &oxideReadTransistor);
				oxideFieldSpecified = true;
				continue;
			}
			if (settingKey.compare(0, strlen("-OxideWriteTransistor"),
					"-OxideWriteTransistor") == 0) {
				ParseAOSParameter(inputFile, settingKey, settingValue,
						"-OxideWriteTransistor", &oxideWriteTransistor);
				oxideFieldSpecified = true;
				continue;
			}
			if (settingKey.compare(0, strlen("-Oxide"), "-Oxide") == 0)
				FailCellFile(inputFile, "unknown AOS setting '" + settingKey + "'");
			if (settingKey == "-RetentionModel") {
				if (retentionModelSpecified)
					FailCellFile(inputFile, "-RetentionModel may be specified only once");
				if (settingValue != "AOSOffCurrentRatio")
					FailCellFile(inputFile, "unsupported -RetentionModel '" + settingValue + "'");
				retentionAOSOffCurrentRatio = true;
				retentionModelSpecified = true;
				continue;
			}
			if (settingKey == "-RetentionReferenceHoldVoltage (V)") {
				if (retentionReferenceHoldVoltageSpecified)
					FailCellFile(inputFile, "-RetentionReferenceHoldVoltage may be specified only once");
				retentionReferenceHoldVoltage = ParseFiniteCellVoltage(
						inputFile, settingKey, settingValue);
				retentionReferenceHoldVoltageSpecified = true;
				continue;
			}
			if (settingKey.compare(0, strlen("-RetentionModel"), "-RetentionModel") == 0
					|| settingKey.compare(0, strlen("-RetentionReferenceHoldVoltage"),
							"-RetentionReferenceHoldVoltage") == 0) {
				FailCellFile(inputFile, "unknown retention setting '" + settingKey + "'");
			}
		}
		if (!strncmp("-MemCellType", line, strlen("-MemCellType"))) {
			sscanf(line, "-MemCellType: %s", tmp);
			if (!strcmp(tmp, "SRAM"))
				memCellType = SRAM;
			else if (!strcmp(tmp, "DRAM"))
				memCellType = DRAM;
			else if (!strcmp(tmp, "eDRAM"))
				memCellType = eDRAM;
			else if (!strcmp(tmp, "gcDRAM"))
				memCellType = gcDRAM;
			else if (!strcmp(tmp, "MRAM"))
				memCellType = MRAM;
			else if (!strcmp(tmp, "PCRAM"))
				memCellType = PCRAM;
			else if (!strcmp(tmp, "FBRAM"))
				memCellType = FBRAM;
			else if (!strcmp(tmp, "memristor"))
				memCellType = memristor;
			else if (!strcmp(tmp, "SLCNAND"))
				memCellType = SLCNAND;
			else if (!strcmp(tmp, "FeRAM"))
				memCellType = FeRAM;
			else
				memCellType = MLCNAND;
			continue;
		}
		if (!strncmp("-ProcessNode", line, strlen("-ProcessNode"))) {
			sscanf(line, "-ProcessNode: %d", &processNode);
			continue;
		}
		if (!strncmp("-CellArea", line, strlen("-CellArea"))) {
			sscanf(line, "-CellArea (F^2): %lf", &area);
			continue;
		}
		if (!strncmp("-CellAspectRatio", line, strlen("-CellAspectRatio"))) {
			sscanf(line, "-CellAspectRatio: %lf", &aspectRatio);
			heightInFeatureSize = sqrt(area * aspectRatio);
			widthInFeatureSize = sqrt(area / aspectRatio);
			continue;
		}

		if (!strncmp("-ResistanceOnAtSetVoltage", line, strlen("-ResistanceOnAtSetVoltage"))) {
			sscanf(line, "-ResistanceOnAtSetVoltage (ohm): %lf", &resistanceOnAtSetVoltage);
			continue;
		}
		if (!strncmp("-ResistanceOffAtSetVoltage", line, strlen("-ResistanceOffAtSetVoltage"))) {
			sscanf(line, "-ResistanceOffAtSetVoltage (ohm): %lf", &resistanceOffAtSetVoltage);
			continue;
		}
		if (!strncmp("-ResistanceOnAtResetVoltage", line, strlen("-ResistanceOnAtResetVoltage"))) {
			sscanf(line, "-ResistanceOnAtResetVoltage (ohm): %lf", &resistanceOnAtResetVoltage);
			continue;
		}
		if (!strncmp("-ResistanceOffAtResetVoltage", line, strlen("-ResistanceOffAtResetVoltage"))) {
			sscanf(line, "-ResistanceOffAtResetVoltage (ohm): %lf", &resistanceOffAtResetVoltage);
			continue;
		}
		if (!strncmp("-ResistanceOnAtReadVoltage", line, strlen("-ResistanceOnAtReadVoltage"))) {
			sscanf(line, "-ResistanceOnAtReadVoltage (ohm): %lf", &resistanceOnAtReadVoltage);
			resistanceOn = resistanceOnAtReadVoltage;
			continue;
		}
		if (!strncmp("-ResistanceOffAtReadVoltage", line, strlen("-ResistanceOffAtReadVoltage"))) {
			sscanf(line, "-ResistanceOffAtReadVoltage (ohm): %lf", &resistanceOffAtReadVoltage);
			resistanceOff = resistanceOffAtReadVoltage;
			continue;
		}
		if (!strncmp("-ResistanceOnAtHalfReadVoltage", line, strlen("-ResistanceOnAtHalfReadVoltage"))) {
			sscanf(line, "-ResistanceOnAtHalfReadVoltage (ohm): %lf", &resistanceOnAtHalfReadVoltage);
			continue;
		}
		if (!strncmp("-ResistanceOffAtHalfReadVoltage", line, strlen("-ResistanceOffAtHalfReadVoltage"))) {
			sscanf(line, "-ResistanceOffAtHalfReadVoltage (ohm): %lf", &resistanceOffAtHalfReadVoltage);
			continue;
		}
		if (!strncmp("-ResistanceOnAtHalfResetVoltage", line, strlen("-ResistanceOnAtHalfResetVoltage"))) {
			sscanf(line, "-ResistanceOnAtHalfResetVoltage (ohm): %lf", &resistanceOnAtHalfResetVoltage);
			continue;
		}

		if (!strncmp("-ResistanceOn", line, strlen("-ResistanceOn"))) {
			sscanf(line, "-ResistanceOn (ohm): %lf", &resistanceOn);
			continue;
		}
		if (!strncmp("-ResistanceOff", line, strlen("-ResistanceOff"))) {
			sscanf(line, "-ResistanceOff (ohm): %lf", &resistanceOff);
			continue;
		}
		if (!strncmp("-CapacitanceOn", line, strlen("-CapacitanceOn"))) {
			sscanf(line, "-CapacitanceOn (F): %lf", &capacitanceOn);
			continue;
		}
		if (!strncmp("-CapacitanceOff", line, strlen("-CapacitanceOff"))) {
			sscanf(line, "-CapacitanceOff (F): %lf", &capacitanceOff);
			continue;
		}

		if (!strncmp("-GateOxThicknessFactor", line, strlen("-GateOxThicknessFactor"))) {
			sscanf(line, "-GateOxThicknessFactor: %lf", &gateOxThicknessFactor);
			continue;
		}

		if (!strncmp("-SOIDeviceWidth (F)", line, strlen("-SOIDeviceWidth (F)"))) {
			sscanf(line, "-SOIDeviceWidth (F): %lf", &widthSOIDevice);
			continue;
		}

		if (!strncmp("-ReadMode", line, strlen("-ReadMode"))) {
			sscanf(line, "-ReadMode: %s", tmp);
			if (!strcmp(tmp, "voltage"))
				readMode = true;
			else
				readMode = false;
			continue;
		}

		if (!strncmp("-ReadVoltage", line, strlen("-ReadVoltage"))) {
			sscanf(line, "-ReadVoltage (V): %lf", &readVoltage);
			continue;
		}
		if (!strncmp("-ReadCurrent", line, strlen("-ReadCurrent"))) {
			sscanf(line, "-ReadCurrent (uA): %lf", &readCurrent);
			readCurrent /= 1e6;
			continue;
		}
		if (!strncmp("-ReadPower", line, strlen("-ReadPower"))) {
			sscanf(line, "-ReadPower (uW): %lf", &readPower);
			readPower /= 1e6;
			continue;
		}
		if (!strncmp("-WordlineBoostRatio", line, strlen("-WordlineBoostRatio"))) {
			sscanf(line, "-WordlineBoostRatio: %lf", &wordlineBoostRatio);
			continue;
		}
		if (!strncmp("-MinSenseVoltage", line, strlen("-MinSenseVoltage"))) {
			sscanf(line, "-MinSenseVoltage (mV): %lf", &minSenseVoltage);
			minSenseVoltage /= 1e3;
			continue;
		}

		if (!strncmp("-ResetMode", line, strlen("-ResetMode"))) {
			sscanf(line, "-ResetMode: %s", tmp);
			if (!strcmp(tmp, "voltage"))
				resetMode = true;
			else
				resetMode = false;
			continue;
		}
		if (!strncmp("-ResetVoltage", line, strlen("-ResetVoltage"))) {
            sscanf(line, "-ResetVoltage (V): %s", tmp);
            if (!strcmp(tmp, "vdd"))
                resetVoltage = tech->vdd;
            else
                sscanf(line, "-ResetVoltage (V): %lf", &resetVoltage);
			continue;
		}
		if (!strncmp("-ResetCurrent", line, strlen("-ResetCurrent"))) {
			sscanf(line, "-ResetCurrent (uA): %lf", &resetCurrent);
			resetCurrent /= 1e6;
			continue;
		}
		if (!strncmp("-ResetPulse", line, strlen("-ResetPulse"))) {
			sscanf(line, "-ResetPulse (ns): %lf", &resetPulse);
			resetPulse /= 1e9;
			continue;
		}
		if (!strncmp("-ResetEnergy", line, strlen("-ResetEnergy"))) {
			sscanf(line, "-ResetEnergy (pJ): %lf", &resetEnergy);
			resetEnergy /= 1e12;
			continue;
		}

		if (!strncmp("-SetMode", line, strlen("-SetMode"))) {
			sscanf(line, "-SetMode: %s", tmp);
			if (!strcmp(tmp, "voltage"))
				setMode = true;
			else
				setMode = false;
			continue;
		}
		if (!strncmp("-SetVoltage", line, strlen("-SetVoltage"))) {
            sscanf(line, "-SetVoltage (V): %s", tmp);
            if (!strcmp(tmp, "vdd"))
                setVoltage = tech->vdd;
            else
                sscanf(line, "-SetVoltage (V): %lf", &setVoltage);
			continue;
		}
		if (!strncmp("-SetCurrent", line, strlen("-SetCurrent"))) {
			sscanf(line, "-SetCurrent (uA): %lf", &setCurrent);
			setCurrent /= 1e6;
			continue;
		}
		if (!strncmp("-SetPulse", line, strlen("-SetPulse"))) {
			sscanf(line, "-SetPulse (ns): %lf", &setPulse);
			setPulse /= 1e9;
			continue;
		}
		if (!strncmp("-SetEnergy", line, strlen("-SetEnergy"))) {
			sscanf(line, "-SetEnergy (pJ): %lf", &setEnergy);
			setEnergy /= 1e12;
			continue;
		}

		if (!strncmp("-Stitching", line, strlen("-Stitching"))) {
			sscanf(line, "-Stitching: %d", &stitching);
			continue;
		}

		if (!strncmp("-AccessType", line, strlen("-AccessType"))) {
			sscanf(line, "-AccessType: %s", tmp);
			if (!strcmp(tmp, "CMOS"))
				accessType = CMOS_access;
			else if (!strcmp(tmp, "BJT"))
				accessType = BJT_access;
			else if (!strcmp(tmp, "diode"))
				accessType = diode_access;
			else
				accessType = none_access;
			continue;
		}

		if (!strncmp("-AccessCMOSWidth", line, strlen("-AccessCMOSWidth"))) {
			if (accessType != CMOS_access)
				cout << "Warning: The input of CMOS access transistor width is ignored because the cell is not CMOS-accessed." << endl;
			else
				sscanf(line, "-AccessCMOSWidth (F): %lf", &widthAccessCMOS);
			continue;
		}

		if (!strncmp("-VoltageDropAccessDevice", line, strlen("-VoltageDropAccessDevice"))) {
			sscanf(line, "-VoltageDropAccessDevice (V): %lf", &voltageDropAccessDevice);
			continue;
		}

		if (!strncmp("-LeakageCurrentAccessDevice", line, strlen("-LeakageCurrentAccessDevice"))) {
			sscanf(line, "-LeakageCurrentAccessDevice (uA): %lf", &leakageCurrentAccessDevice);
			leakageCurrentAccessDevice /= 1e6;
			continue;
		}

		if (!strncmp("-DRAMCellCapacitance", line, strlen("-DRAMCellCapacitance"))) {
			if (memCellType != DRAM && memCellType != eDRAM && memCellType != gcDRAM && memCellType != FeRAM)
				cout << "Warning: The input of DRAM cell capacitance is ignored because the memory cell is not DRAM." << endl;
			else
				sscanf(line, "-DRAMCellCapacitance (F): %lf", &capDRAMCell);
			continue;
		}

		if (!strncmp("-SRAMCellNMOSWidth", line, strlen("-SRAMCellNMOSWidth"))) {
			if (memCellType != SRAM)
				cout << "Warning: The input of SRAM cell NMOS width is ignored because the memory cell is not SRAM." << endl;
			else
				sscanf(line, "-SRAMCellNMOSWidth (F): %lf", &widthSRAMCellNMOS);
			continue;
		}

		if (!strncmp("-SRAMCellPMOSWidth", line, strlen("-SRAMCellPMOSWidth"))) {
			if (memCellType != SRAM)
				cout << "Warning: The input of SRAM cell PMOS width is ignored because the memory cell is not SRAM." << endl;
			else
				sscanf(line, "-SRAMCellPMOSWidth (F): %lf", &widthSRAMCellPMOS);
			continue;
		}


		if (!strncmp("-ReadFloating", line, strlen("-ReadFloating"))) {
			sscanf(line, "-ReadFloating: %s", tmp);
			if (!strcmp(tmp, "true"))
				readFloating = true;
			else
				readFloating = false;
			continue;
		}

		if (!strncmp("-FlashEraseVoltage (V)", line, strlen("-FlashEraseVoltage (V)"))) {
			if (memCellType != SLCNAND && memCellType != MLCNAND)
				cout << "Warning: The input of programming/erase voltage is ignored because the memory cell is not flash." << endl;
			else
				sscanf(line, "-FlashEraseVoltage (V): %lf", &flashEraseVoltage);
			continue;
		}

		if (!strncmp("-FlashProgramVoltage (V)", line, strlen("-FlashProgramVoltage (V)"))) {
			if (memCellType != SLCNAND && memCellType != MLCNAND)
				cout << "Warning: The input of programming/program voltage is ignored because the memory cell is not flash." << endl;
			else
				sscanf(line, "-FlashProgramVoltage (V): %lf", &flashProgramVoltage);
			continue;
		}

		if (!strncmp("-FlashPassVoltage (V)", line, strlen("-FlashPassVoltage (V)"))) {
			if (memCellType != SLCNAND && memCellType != MLCNAND)
				cout << "Warning: The input of pass voltage is ignored because the memory cell is not flash." << endl;
			else
				sscanf(line, "-FlashPassVoltage (V): %lf", &flashPassVoltage);
			continue;
		}

		if (!strncmp("-FlashEraseTime", line, strlen("-FlashEraseTime"))) {
			if (memCellType != SLCNAND && memCellType != MLCNAND)
				cout << "Warning: The input of erase time is ignored because the memory cell is not flash." << endl;
			else {
				sscanf(line, "-FlashEraseTime (ms): %lf", &flashEraseTime);
				flashEraseTime /= 1e3;
			}
			continue;
		}

		if (!strncmp("-FlashProgramTime", line, strlen("-FlashProgramTime"))) {
			if (memCellType != SLCNAND && memCellType != MLCNAND)
				cout << "Warning: The input of erase time is ignored because the memory cell is not flash." << endl;
			else {
				sscanf(line, "-FlashProgramTime (us): %lf", &flashProgramTime);
				flashProgramTime /= 1e6;
			}
			continue;
		}

		if (!strncmp("-GateCouplingRatio", line, strlen("-GateCouplingRatio"))) {
			if (memCellType != SLCNAND && memCellType != MLCNAND)
				cout << "Warning: The input of gate coupling ratio (GCR) is ignored because the memory cell is not flash." << endl;
			else {
				sscanf(line, "-GateCouplingRatio: %lf", &gateCouplingRatio);
			}
			continue;
		}

		if (!strncmp("-RetentionTime", line, strlen("-RetentionTime"))) {
			sscanf(line, "-RetentionTime (us): %lf", &retentionTime);
            retentionTime /= 1e6;
			
			continue;
		}

		if (!strncmp("-Temperature", line, strlen("-Temperature"))) {
			if (memCellType != eDRAM && memCellType != gcDRAM)
				cout << "Warning: The input of temperature is ignored because the cell is not eDRAM." << endl;
			else
				sscanf(line, "-Temperature (K): %lf", &temperature);
			continue;
		}
	}

	fclose(fp);

	if (oxideFieldSpecified && (!oxideControlSpecified || !oxideTransistor))
		FailCellFile(inputFile, "AOS transistor fields require -OxideTransistor: true");
	if (oxideTransistor && !IsAOSCell(memCellType))
		FailCellFile(inputFile, "-OxideTransistor is supported only for eDRAM and gcDRAM");
	if (oxideTransistor && memCellType == eDRAM) {
		if (oxideReadTransistor.initialized || oxideWriteTransistor.initialized)
			FailCellFile(inputFile, "eDRAM accepts only -OxideAccessTransistor fields");
		ValidateAOSParameterSet(inputFile, "eDRAM access", oxideAccessTransistor);
	}
	if (oxideTransistor && memCellType == gcDRAM) {
		if (oxideAccessTransistor.initialized)
			FailCellFile(inputFile, "gcDRAM accepts only read/write AOS transistor fields");
		ValidateAOSParameterSet(inputFile, "gcDRAM read", oxideReadTransistor);
		ValidateAOSParameterSet(inputFile, "gcDRAM write", oxideWriteTransistor);
	}
	if (retentionAOSOffCurrentRatio) {
		if (memCellType != eDRAM || !oxideTransistor)
			FailCellFile(inputFile, "AOSOffCurrentRatio retention is supported only for AOS eDRAM");
		if (retentionTime == invalid_value)
			FailCellFile(inputFile, "AOSOffCurrentRatio requires a supplied -RetentionTime");
		if (!retentionReferenceHoldVoltageSpecified)
			FailCellFile(inputFile, "AOSOffCurrentRatio requires -RetentionReferenceHoldVoltage");
	}
	if (retentionReferenceHoldVoltageSpecified && !retentionAOSOffCurrentRatio)
		FailCellFile(inputFile, "-RetentionReferenceHoldVoltage is unused without -RetentionModel: AOSOffCurrentRatio");
}


void MemCell::ApplyPVT() {
    if (retentionTime == invalid_value) {
        /* TODO: No given retention time, we should calculate it. */
        return;
    }

    if (memCellType == eDRAM || memCellType == gcDRAM) {
        cout << "[Info] Retention time given at " << temperature << "K is " << retentionTime * 1e6 << "us" << endl;
		if (retentionAOSOffCurrentRatio) {
			try {
				AOSFETCompactModel model(oxideAccessTransistor.parameters);
				double referenceCurrent = model.CalculateDrainCurrentExternal(
						retentionReferenceHoldVoltage, tech->vdd);
				/* The configured-bias current was validated and cached when the
				 * cell schema was parsed.  Only the optional reference bias needs
				 * another compact-model evaluation. */
				double appliedCurrent = oxideAccessTransistor.operatingPoint.Ioff;
				if (!std::isfinite(referenceCurrent) || !std::isfinite(appliedCurrent)
						|| referenceCurrent <= 0 || appliedCurrent <= 0) {
					throw std::runtime_error("invalid off current");
				}
				double currentRatio = referenceCurrent / appliedCurrent;
				retentionTime *= currentRatio;
				cout << "[Info] AOSOffCurrentRatio retention: reference/applied Ioff ratio = "
						<< currentRatio << endl;
			} catch (const std::exception &error) {
				cerr << "AOS retention evaluation failed: " << error.what() << endl;
				exit(-1);
			}
		}
        double exponent = -0.0268 * (inputParameter->temperature - temperature);
        retentionTime = retentionTime * exp(exponent);
		if (!std::isfinite(retentionTime) || retentionTime <= 0) {
			cerr << "Retention calculation produced an invalid value." << endl;
			exit(-1);
		}
        cout << "[Info] Retention time at " << inputParameter->temperature << "K is " << retentionTime * 1e6 << "us" << endl;
    }
}


void MemCell::CellScaling(int _targetProcessNode) {
	if ((processNode > 0) && (processNode != _targetProcessNode)) {
		double scalingFactor = (double)processNode / _targetProcessNode;
		if (memCellType == PCRAM) {
			resistanceOn *= scalingFactor;
			resistanceOff *= scalingFactor;
			if (!setMode) {
				setCurrent /= scalingFactor;
			} else {
				setVoltage *= 1;
			}
			if (!resetMode) {
				resetCurrent /= scalingFactor;
			} else {
				resetVoltage *= 1;
			}
			if (accessType == diode_access) {
				capacitanceOn /= scalingFactor; //TO-DO
				capacitanceOff /= scalingFactor; //TO-DO
			}
		} else if (memCellType == MRAM){ //TO-DO: MRAM
			resistanceOn *= scalingFactor * scalingFactor;
			resistanceOff *= scalingFactor * scalingFactor;
			if (!setMode) {
				setCurrent /= scalingFactor;
			} else {
				setVoltage *= scalingFactor;
			}
			if (!resetMode) {
				resetCurrent /= scalingFactor;
			} else {
				resetVoltage *= scalingFactor;
			}
			if (accessType == diode_access) {
				capacitanceOn /= scalingFactor; //TO-DO
				capacitanceOff /= scalingFactor; //TO-DO
			}
		} else if (memCellType == memristor) { //TO-DO: memristor

		} else { //TO-DO: other RAMs

		}
		processNode = _targetProcessNode;
	}
}

double MemCell::GetMemristance(double _relativeReadVoltage) { /* Get the LRS resistance of memristor at log-linera region of I-V curve */
	if (memCellType == memristor) {
		double x1, x2, x3;  // x1: read voltage, x2: half voltage, x3: applied voltage
		if (readVoltage == 0) {
			x1 = readCurrent * resistanceOnAtReadVoltage;
		} else {
			x1 = readVoltage;
		}
		x2 = readVoltage / 2;
		x3 = _relativeReadVoltage * readVoltage;
		double y1, y2 ,y3; // y1:log(read current), y2: log(leakage current at half read voltage
		y1 = log2(x1/resistanceOnAtReadVoltage);
		y2 = log2(x2/resistanceOnAtHalfReadVoltage);
		y3 = (y2 - y1) / (x2 -x1) * x3 + (x2 * y1 - x1 * y2) / (x2 - x1);  //insertion
		return x3 / pow(2, y3);
	} else {  // not memristor, can't call the function
		cout <<"Warning[MemCell] : Try to get memristance from a non-memristor memory cell" << endl;
		return -1;
	}
}

void MemCell::CalculateWriteEnergy() {
	if (resetEnergy == 0) {
		if (resetMode) {
			if (memCellType == memristor)
				if (accessType == none_access)
					resetEnergy = fabs(resetVoltage) * (fabs(resetVoltage) - voltageDropAccessDevice) / resistanceOnAtResetVoltage * resetPulse;
				else
					resetEnergy = fabs(resetVoltage) * (fabs(resetVoltage) - voltageDropAccessDevice) / resistanceOn * resetPulse;
			else if (memCellType == PCRAM)
				resetEnergy = fabs(resetVoltage) * (fabs(resetVoltage) - voltageDropAccessDevice) / resistanceOn * resetPulse;	// PCM cells shows low resistance during most time of the switching
			else if (memCellType == FBRAM)
				resetEnergy = fabs(resetVoltage) * fabs(resetCurrent) * resetPulse;
			else
				resetEnergy = fabs(resetVoltage) * (fabs(resetVoltage) - voltageDropAccessDevice) / resistanceOn * resetPulse;
		} else {
			if (resetVoltage == 0){
				resetEnergy = tech->vdd * fabs(resetCurrent) * resetPulse; /*TO-DO consider charge pump*/
			} else {
				resetEnergy = fabs(resetVoltage) * fabs(resetCurrent) * resetPulse;
			}
			/* previous model seems to be problematic
			if (memCellType == memristor)
				if (accessType == none_access)
					resetEnergy = resetCurrent * (resetCurrent * resistanceOffAtResetVoltage + voltageDropAccessDevice) * resetPulse;
				else
					resetEnergy = resetCurrent * (resetCurrent * resistanceOff + voltageDropAccessDevice) * resetPulse;
			else if (memCellType == PCRAM)
				resetEnergy = resetCurrent * (resetCurrent * resistanceOn + voltageDropAccessDevice) * resetPulse;		// PCM cells shows low resistance during most time of the switching
			else if (memCellType == FBRAM)
				resetEnergy = fabs(resetVoltage) * fabs(resetCurrent) * resetPulse;
			else
				resetEnergy = resetCurrent * (resetCurrent * resistanceOff + voltageDropAccessDevice) * resetPulse;
		    */
		}
	}
	if (setEnergy == 0) {
		if (setMode) {
			if (memCellType == memristor)
				if (accessType == none_access)
					setEnergy = fabs(setVoltage) * (fabs(setVoltage) - voltageDropAccessDevice) / resistanceOnAtSetVoltage * setPulse;
				else
					setEnergy = fabs(setVoltage) * (fabs(setVoltage) - voltageDropAccessDevice) / resistanceOn * setPulse;
			else if (memCellType == PCRAM)
				setEnergy = fabs(setVoltage) * (fabs(setVoltage) - voltageDropAccessDevice) / resistanceOn * setPulse;			// PCM cells shows low resistance during most time of the switching
			else if (memCellType == FBRAM)
				setEnergy = fabs(setVoltage) * fabs(setCurrent) * setPulse;
			else
				setEnergy = fabs(setVoltage) * (fabs(setVoltage) - voltageDropAccessDevice) / resistanceOn * setPulse;
		} else {
			if (resetVoltage == 0){
				setEnergy = tech->vdd * fabs(setCurrent) * setPulse; /*TO-DO consider charge pump*/
			} else {
				setEnergy = fabs(setVoltage) * fabs(setCurrent) * setPulse;
			}
			/* previous model seems to be problematic
			if (memCellType == memristor)
				if (accessType == none_access)
					setEnergy = setCurrent * (setCurrent * resistanceOffAtSetVoltage + voltageDropAccessDevice) * setPulse;
				else
					setEnergy = setCurrent * (setCurrent * resistanceOff + voltageDropAccessDevice) * setPulse;
			else if (memCellType == PCRAM)
				setEnergy = setCurrent * (setCurrent * resistanceOn + voltageDropAccessDevice) * setPulse;		// PCM cells shows low resistance during most time of the switching
			else if (memCellType == FBRAM)
				setEnergy = fabs(setVoltage) * fabs(setCurrent) * setPulse;
			else
				setEnergy = setCurrent * (setCurrent * resistanceOff + voltageDropAccessDevice) * setPulse;
			*/
		}
	}
}

double MemCell::CalculateReadPower() { /* TO-DO consider charge pumped read voltage */
	if (readPower == 0) {
		if (cell->readMode) {	/* voltage-sensing */
			if (readVoltage == 0) { /* Current-in voltage sensing */
				return tech->vdd * readCurrent;
			}
			if (readCurrent == 0) { /*Voltage-divider sensing */
				double resInSerialForSenseAmp, maxBitlineCurrent;
				resInSerialForSenseAmp = sqrt(resistanceOn * resistanceOff);
				maxBitlineCurrent = (readVoltage - voltageDropAccessDevice) / (resistanceOn + resInSerialForSenseAmp);
				return tech->vdd * maxBitlineCurrent;
			}
		} else { /* current-sensing */
			double maxBitlineCurrent = (readVoltage - voltageDropAccessDevice) / resistanceOn;
			return tech->vdd * maxBitlineCurrent;
		}
	} else {
		return -1.0; /* should not call the function if read energy exists */
	}
	return -1.0;
}

void MemCell::PrintCell(int indent)
{
	switch (memCellType) {
	case SRAM:
		cout << string(indent, ' ') << "Memory Cell: SRAM" << endl;
		break;
	case DRAM:
		cout << string(indent, ' ') << "Memory Cell: DRAM" << endl;
		break;
	case eDRAM:
		cout << string(indent, ' ') << "Memory Cell: Embedded DRAM" << endl;
		break;
	case gcDRAM:
		cout << string(indent, ' ') << "Memory Cell: Gain Cell DRAM" << endl;
		break;
	case MRAM:
		cout << string(indent, ' ') << "Memory Cell: MRAM (Magnetoresistive)" << endl;
		break;
	case PCRAM:
		cout << string(indent, ' ') << "Memory Cell: PCRAM (Phase-Change)" << endl;
		break;
	case memristor:
		cout << string(indent, ' ') << "Memory Cell: RRAM (Memristor)" << endl;
		break;
	case FBRAM:
		cout << string(indent, ' ') << "Memory Cell: FBRAM (Floating Body)" <<endl;
		break;
	case SLCNAND:
		cout << string(indent, ' ') << "Memory Cell: Single-Level Cell NAND Flash" << endl;
		break;
	case MLCNAND:
		cout << string(indent, ' ') << "Memory Cell: Multi-Level Cell NAND Flash" << endl;
		break;
	default:
		cout << string(indent, ' ') << "Memory Cell: Unknown" << endl;
	}
	cout << string(indent, ' ') << "Cell Area (F^2)    : " << area << " (" << heightInFeatureSize << "Fx" << widthInFeatureSize << "F)" << endl;
	cout << string(indent, ' ') << "Cell Aspect Ratio  : " << aspectRatio << endl;

	if (memCellType == PCRAM || memCellType == MRAM || memCellType == memristor || memCellType == FBRAM) {
		if (resistanceOn < 1e3 )
			cout << string(indent, ' ') << "Cell Turned-On Resistance : " << resistanceOn << "ohm" << endl;
		else if (resistanceOn < 1e6)
			cout << string(indent, ' ') << "Cell Turned-On Resistance : " << resistanceOn / 1e3 << "Kohm" << endl;
		else
			cout << string(indent, ' ') << "Cell Turned-On Resistance : " << resistanceOn / 1e6 << "Mohm" << endl;
		if (resistanceOff < 1e3 )
			cout << string(indent, ' ') << "Cell Turned-Off Resistance: "<< resistanceOff << "ohm" << endl;
		else if (resistanceOff < 1e6)
			cout << string(indent, ' ') << "Cell Turned-Off Resistance: "<< resistanceOff / 1e3 << "Kohm" << endl;
		else
			cout << string(indent, ' ') << "Cell Turned-Off Resistance: "<< resistanceOff / 1e6 << "Mohm" << endl;

		if (readMode) {
			cout << string(indent, ' ') << "Read Mode: Voltage-Sensing" << endl;
			if (readCurrent > 0)
				cout << string(indent, ' ') << "  - Read Current: " << readCurrent * 1e6 << "uA" << endl;
			if (readVoltage > 0)
				cout << string(indent, ' ') << "  - Read Voltage: " << readVoltage << "V" << endl;
		} else {
			cout << string(indent, ' ') << "Read Mode: Current-Sensing" << endl;
			if (readCurrent > 0)
				cout << string(indent, ' ') << "  - Read Current: " << readCurrent * 1e6 << "uA" << endl;
			if (readVoltage > 0)
				cout << string(indent, ' ') << "  - Read Voltage: " << readVoltage << "V" << endl;
		}

		if (resetMode) {
			cout << string(indent, ' ') << "Reset Mode: Voltage" << endl;
			cout << string(indent, ' ') << "  - Reset Voltage: " << resetVoltage << "V" << endl;
		} else {
			cout << string(indent, ' ') << "Reset Mode: Current" << endl;
			cout << string(indent, ' ') << "  - Reset Current: " << resetCurrent * 1e6 << "uA" << endl;
		}
		cout << string(indent, ' ') << "  - Reset Pulse: " << TO_SECOND(resetPulse) << endl;

		if (setMode) {
			cout << string(indent, ' ') << "Set Mode: Voltage" << endl;
			cout << string(indent, ' ') << "  - Set Voltage: " << setVoltage << "V" << endl;
		} else {
			cout << string(indent, ' ') << "Set Mode: Current" << endl;
			cout << string(indent, ' ') << "  - Set Current: " << setCurrent * 1e6 << "uA" << endl;
		}
		cout << string(indent, ' ') << "  - Set Pulse: " << TO_SECOND(setPulse) << endl;

		switch (accessType) {
		case CMOS_access:
			cout << string(indent, ' ') << "Access Type: CMOS" << endl;
			break;
		case BJT_access:
			cout << string(indent, ' ') << "Access Type: BJT" << endl;
			break;
		case diode_access:
			cout << string(indent, ' ') << "Access Type: Diode" << endl;
			break;
		default:
			cout << string(indent, ' ') << "Access Type: None Access Device" << endl;
		}
	} else if (memCellType == SRAM) {
		cout << string(indent, ' ') << "SRAM Cell Access Transistor Width: " << widthAccessCMOS << "F" << endl;
		cout << string(indent, ' ') << "SRAM Cell NMOS Width: " << widthSRAMCellNMOS << "F" << endl;
		cout << string(indent, ' ') << "SRAM Cell PMOS Width: " << widthSRAMCellPMOS << "F" << endl;
	} else if (memCellType == SLCNAND) {
		cout << string(indent, ' ') << "Pass Voltage       : " << flashPassVoltage << "V" << endl;
		cout << string(indent, ' ') << "Programming Voltage: " << flashProgramVoltage << "V" << endl;
		cout << string(indent, ' ') << "Erase Voltage      : " << flashEraseVoltage << "V" << endl;
		cout << string(indent, ' ') << "Programming Time   : " << TO_SECOND(flashProgramTime) << endl;
		cout << string(indent, ' ') << "Erase Time         : " << TO_SECOND(flashEraseTime) << endl;
		cout << string(indent, ' ') << "Gate Coupling Ratio: " << gateCouplingRatio << endl;
	}

	if (oxideTransistor && memCellType == eDRAM) {
		PrintAOSParameterSet(indent, "Access", oxideAccessTransistor);
	} else if (oxideTransistor && memCellType == gcDRAM) {
		PrintAOSParameterSet(indent, "Read", oxideReadTransistor);
		PrintAOSParameterSet(indent, "Write", oxideWriteTransistor);
	}
}
