#include "AOSFETCompactModel.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace {

void Require(bool condition, const char *message) {
	if (!condition)
		throw std::runtime_error(message);
}

} // namespace

int main() {
	try {
		AOSFETCompactModel model;
		const double points[][2] = {
			{-0.2, 1.0}, {0.15, 1.0}, {0.5, 0.1},
			{1.0, 1.0}, {1.5, 1.0}, {1.1, 1.0}
		};
		std::cout << std::setprecision(17);
		for (const auto &point : points) {
			AOSFETCompactModel::Capacitances caps =
					model.CalculateCapacitances(point[0], point[1]);
			std::cout << point[0] << " " << point[1] << " "
					<< model.Mobility(point[0]) << " "
					<< model.SurfacePotential(point[0], point[1]) << " "
					<< model.CalculateDrainCurrent(point[0], point[1]) << " "
					<< caps.gateSource << " " << caps.gateDrain << "\n";
		}

		AOSFETCompactModel::Parameters base = AOSFETCompactModel::DefaultParameters();
		base.useConstantMobility = true;
		base.mobility = 20e-4;
		AOSFETCompactModel noContact(base);
		AOSDeviceOperatingPoint reference = noContact.EvaluateOperatingPoint(1.0, 0.0, 1.0);
		Require(reference.Ion > reference.Ioff && reference.Cgate >= reference.Cdrain,
				"invalid operating point");

		AOSFETCompactModel::Parameters contactParams = base;
		contactParams.totalContactResistanceKohmUm = 10.0;
		AOSDeviceOperatingPoint contact =
				AOSFETCompactModel(contactParams).EvaluateOperatingPoint(1.0, 0.0, 1.0);
		Require(contact.Ion < reference.Ion, "contact resistance must reduce Ion");

		AOSFETCompactModel::Parameters wideParams = base;
		wideParams.width *= 2;
		double wideCurrent = AOSFETCompactModel(wideParams).CalculateDrainCurrent(1.0, 1.0);
		Require(std::fabs(wideCurrent / reference.Ion - 2.0) < 1e-9,
				"current must scale with width");

		AOSFETCompactModel::Parameters shortParams = base;
		shortParams.length /= 2;
		double shortCurrent = AOSFETCompactModel(shortParams).CalculateDrainCurrent(1.0, 1.0);
		Require(std::fabs(shortCurrent / reference.Ion - 2.0) < 1e-9,
				"current must scale inversely with length");

		AOSFETCompactModel::Parameters mobilityParams = base;
		mobilityParams.mobilityScale = 0.5;
		double mobilityScaledCurrent = AOSFETCompactModel(mobilityParams)
				.CalculateDrainCurrent(1.0, 1.0);
		Require(std::fabs(mobilityScaledCurrent / reference.Ion - 0.5) < 1e-9,
				"mobility scale must scale on current");

		AOSFETCompactModel::Parameters leakageParams = base;
		leakageParams.leakageScale = 2;
		leakageParams.currentFloor = 0;
		double leakage = AOSFETCompactModel(leakageParams).CalculateDrainCurrent(0.0, 1.0);
		AOSFETCompactModel::Parameters leakageReferenceParams = base;
		leakageReferenceParams.currentFloor = 0;
		double leakageReference = AOSFETCompactModel(leakageReferenceParams)
				.CalculateDrainCurrent(0.0, 1.0);
		Require(std::fabs(leakage / leakageReference - 2.0) < 1e-9,
				"leakage scale must scale off current");
		Require(noContact.CalculateDrainCurrent(-0.1, 1.0)
				< noContact.CalculateDrainCurrent(0.0, 1.0),
				"lower hold voltage must reduce Ioff and increase ratio-scaled retention");

		Require(std::isfinite(model.CalculateLeakageCurrent(-100.0, 1.0)),
				"bounded negative exponential must remain finite");
		Require(std::isfinite(model.CalculateLeakageCurrent(100.0, 1.0)),
				"bounded positive exponential must remain finite");

		bool invalidTemperatureRejected = false;
		try {
			AOSFETCompactModel::Parameters invalid = base;
			invalid.temperature = invalid.tailTemperature;
			AOSFETCompactModel rejected(invalid);
			(void)rejected;
		} catch (const std::invalid_argument &) {
			invalidTemperatureRejected = true;
		}
		Require(invalidTemperatureRejected, "T >= tail temperature must be rejected");

		bool nonconvergenceRejected = false;
		try {
			model.SurfacePotential(1.0, 1.0, 1, 1e-30);
		} catch (const std::runtime_error &) {
			nonconvergenceRejected = true;
		}
		Require(nonconvergenceRejected, "non-convergence must be reported");
	} catch (const std::exception &error) {
		std::cerr << error.what() << "\n";
		return 1;
	}
	return 0;
}
