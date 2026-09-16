/*******************************************************************************
* Copyright (c) 2025
* Georgia Institute of Technology
*
* This source code is part of NeuroSim (NS)-Cache.
*
* The AOS FET compact model is based on work by Jay Sonawane.
*******************************************************************************/

#include "AOSFETCompactModel.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace {

const double kPi = 3.141592653589793238462643383279502884;

void RequireFinite(double value, const char *name) {
	if (!std::isfinite(value)) {
		throw std::invalid_argument(std::string("AOS compact-model parameter '")
				+ name + "' must be finite");
	}
}

void RequirePositive(double value, const char *name) {
	RequireFinite(value, name);
	if (value <= 0) {
		throw std::invalid_argument(std::string("AOS compact-model parameter '")
				+ name + "' must be greater than zero");
	}
}

void RequireNonnegative(double value, const char *name) {
	RequireFinite(value, name);
	if (value < 0) {
		throw std::invalid_argument(std::string("AOS compact-model parameter '")
				+ name + "' must not be negative");
	}
}

void RequireFiniteResult(double value, const char *name) {
	if (!std::isfinite(value)) {
		throw std::runtime_error(std::string("AOS compact model produced non-finite ") + name);
	}
}

} // namespace

AOSFETCompactModel::AOSFETCompactModel() {
	Initialize(DefaultParameters());
}

AOSFETCompactModel::AOSFETCompactModel(const Parameters &_parameters) {
	Initialize(_parameters);
}

void AOSFETCompactModel::Initialize(const Parameters &_parameters) {
	parameters = _parameters;
	ValidateParameters();
	UpdateDerivedParameters();
}

AOSFETCompactModel::Parameters AOSFETCompactModel::DefaultParameters() {
	Parameters params;
	params.q = 1.602e-19;
	params.k = 1.380649e-23;
	params.temperature = 300.0;
	params.tailTemperature = 400.0;
	params.eps0 = 8.854e-12;
	params.semiconductorPermittivity = params.eps0 * 10;
	params.nu0T0 = 1.0;
	params.flatBandVoltage = 0.15;
	params.width = 1e-6;
	params.length = 2e-6;
	params.trapDensity = 1e17;
	params.fermiReference = 0;
	params.oxideCapacitance = 8.854e-12 * 21 / 5e-9;
	params.overlapCapacitance = 30e-18;
	params.vrhThreshold = -1.5;
	params.percolationThreshold = -1.0;
	params.vrhPrefactor = 15.0;
	params.percolationPrefactor = 35.0;
	params.percolationExponent = 0.3;
	params.useConstantMobility = false;
	params.mobility = 0.0;
	params.mobilityScale = 1.0;
	params.leakageSwingFactor = 2.0;
	params.leakageSurfacePotentialReference = 0.15;
	params.leakageScale = 1.0;
	params.currentFloor = 1e-17;
	params.totalContactResistanceKohmUm = 0.0;
	return params;
}

void AOSFETCompactModel::ValidateParameters() const {
	RequirePositive(parameters.q, "q");
	RequirePositive(parameters.k, "k");
	RequirePositive(parameters.temperature, "temperature");
	RequirePositive(parameters.tailTemperature, "tail temperature");
	if (parameters.temperature >= parameters.tailTemperature) {
		throw std::invalid_argument(
				"AOS compact model requires 0 < operating temperature < tail temperature");
	}
	RequirePositive(parameters.eps0, "vacuum permittivity");
	RequirePositive(parameters.semiconductorPermittivity, "semiconductor permittivity");
	RequirePositive(parameters.nu0T0, "nu0T0");
	RequireFinite(parameters.flatBandVoltage, "flat-band voltage");
	RequirePositive(parameters.width, "width");
	RequirePositive(parameters.length, "length");
	RequirePositive(parameters.trapDensity, "trap density");
	RequireFinite(parameters.fermiReference, "Fermi reference");
	RequirePositive(parameters.oxideCapacitance, "oxide capacitance");
	RequireNonnegative(parameters.overlapCapacitance, "overlap capacitance");
	RequireFinite(parameters.vrhThreshold, "VRH threshold");
	RequireFinite(parameters.percolationThreshold, "percolation threshold");
	RequirePositive(parameters.vrhPrefactor, "VRH prefactor");
	RequirePositive(parameters.percolationPrefactor, "percolation prefactor");
	RequirePositive(parameters.percolationExponent, "percolation exponent");
	if (parameters.useConstantMobility) {
		RequirePositive(parameters.mobility, "constant mobility");
	} else {
		RequireNonnegative(parameters.mobility, "constant mobility");
	}
	RequirePositive(parameters.mobilityScale, "mobility scale");
	RequirePositive(parameters.leakageSwingFactor, "leakage swing factor");
	RequirePositive(parameters.leakageSurfacePotentialReference,
			"leakage surface-potential reference");
	RequirePositive(parameters.leakageScale, "leakage scale");
	RequireNonnegative(parameters.currentFloor, "current floor");
	RequireNonnegative(parameters.totalContactResistanceKohmUm,
			"total contact resistance");
}

void AOSFETCompactModel::UpdateDerivedParameters() {
	mobilityVrhExponent = 2 * ((parameters.tailTemperature / parameters.temperature) - 1);
	voltageNormalization = 2 * parameters.k * parameters.tailTemperature / parameters.q;
	thermalVoltage = parameters.k * parameters.temperature / parameters.q;

	double z = parameters.temperature / parameters.tailTemperature;
	double gammaDenominator = std::sin(kPi * z);
	if (!std::isfinite(gammaDenominator) || gammaDenominator <= 0) {
		throw std::invalid_argument("AOS compact model gamma term is singular");
	}
	double gammaProduct = (kPi * z) / gammaDenominator;
	double trapTerm = parameters.q * parameters.semiconductorPermittivity
			* parameters.trapDensity * (parameters.k * parameters.tailTemperature / parameters.q)
			* gammaProduct * LimExp(parameters.fermiReference
					/ (parameters.k * parameters.tailTemperature));
	double deepStateTerm = parameters.q * parameters.semiconductorPermittivity
			* parameters.nu0T0 * parameters.trapDensity
			* (parameters.k * parameters.temperature / parameters.q)
			* LimExp(parameters.fermiReference / (parameters.k * parameters.temperature));
	if (!(trapTerm > 0) || !(deepStateTerm > 0)
			|| !std::isfinite(trapTerm) || !std::isfinite(deepStateTerm)) {
		throw std::invalid_argument("AOS compact model calibration produced an invalid state coefficient");
	}
	trapCoefficient = std::sqrt(trapTerm) / parameters.oxideCapacitance;
	deepStateCoefficient = std::sqrt(deepStateTerm) / parameters.oxideCapacitance;

	double referenceMobility = Mobility(std::max(parameters.flatBandVoltage, 0.0));
	double leakageSqrtTerm = std::sqrt((parameters.q * parameters.semiconductorPermittivity
			* parameters.trapDensity)
			/ (2 * parameters.leakageSurfacePotentialReference));
	leakagePrefactor = (parameters.width / parameters.length) * referenceMobility
			* leakageSqrtTerm * thermalVoltage * thermalVoltage;
	RequireFiniteResult(mobilityVrhExponent, "mobility exponent");
	RequireFiniteResult(voltageNormalization, "voltage normalization");
	RequireFiniteResult(thermalVoltage, "thermal voltage");
	RequireFiniteResult(trapCoefficient, "trap coefficient");
	RequireFiniteResult(deepStateCoefficient, "deep-state coefficient");
	RequireFiniteResult(leakagePrefactor, "leakage prefactor");
	if (referenceMobility <= 0 || leakagePrefactor <= 0) {
		throw std::invalid_argument("AOS compact model derived mobility/current scale must be positive");
	}
}

double AOSFETCompactModel::Mobility(double _vgs) const {
	RequireFinite(_vgs, "Vgs");
	double mobility;
	if (parameters.useConstantMobility) {
		mobility = parameters.mobility;
	} else {
		double baseVrh = std::max(_vgs - parameters.vrhThreshold, 1e-9);
		double basePercolation = std::max(_vgs - parameters.percolationThreshold, 1e-9);
		double mobilityVrhCm = parameters.vrhPrefactor
				* std::pow(baseVrh, mobilityVrhExponent);
		double mobilityPercolationCm = parameters.percolationPrefactor
				* std::pow(basePercolation, parameters.percolationExponent);
		double denominator = mobilityVrhCm + mobilityPercolationCm;
		if (!(denominator > 0) || !std::isfinite(denominator)) {
			throw std::runtime_error("AOS compact model produced invalid mobility components");
		}
		mobility = (mobilityVrhCm * mobilityPercolationCm / denominator) * 1e-4;
	}
	mobility *= parameters.mobilityScale;
	RequireFiniteResult(mobility, "mobility");
	if (mobility <= 0) {
		throw std::runtime_error("AOS compact model produced nonpositive mobility");
	}
	return mobility;
}

double AOSFETCompactModel::SurfacePotential(double _vgs, double _vch,
		int _maxIteration, double _tolerance) const {
	RequireFinite(_vgs, "Vgs");
	RequireFinite(_vch, "channel voltage");
	RequirePositive(_tolerance, "surface-potential tolerance");
	if (_maxIteration <= 0) {
		throw std::invalid_argument("AOS surface-potential iteration limit must be positive");
	}
	if (_vgs < parameters.flatBandVoltage) {
		return _vgs - parameters.flatBandVoltage;
	}

	double x = InitialSurfacePotentialGuess(_vgs, _vch);
	double normalizedTrapCoefficient = trapCoefficient / voltageNormalization;
	double normalizedDeepStateCoefficient = deepStateCoefficient / voltageNormalization;
	double normalizedGate = GateNormalized(_vgs);
	double normalizedDrain = DrainNormalized(_vch);
	double temperatureRatio = parameters.tailTemperature / parameters.temperature;

	for (int i = 0; i < _maxIteration; ++i) {
		double expDeep = LimExp(x - normalizedDrain);
		double expTail = LimExp((x - normalizedDrain) * temperatureRatio);
		double f = (normalizedGate - x) - normalizedTrapCoefficient * expDeep
				- normalizedDeepStateCoefficient * expTail;
		double df = -1 - normalizedTrapCoefficient * expDeep
				- normalizedDeepStateCoefficient * temperatureRatio * expTail;
		double d2f = -normalizedTrapCoefficient * expDeep
				- normalizedDeepStateCoefficient * temperatureRatio * temperatureRatio * expTail;
		if (!std::isfinite(f) || !std::isfinite(df) || !std::isfinite(d2f)
				|| std::fabs(df) < 1e-14) {
			throw std::runtime_error("AOS surface-potential solver became singular");
		}
		double correction = (f / df) * (1 + (d2f * f) / (2 * df * df));
		if (!std::isfinite(correction)) {
			throw std::runtime_error("AOS surface-potential solver diverged");
		}
		x -= correction;
		if (!std::isfinite(x)) {
			throw std::runtime_error("AOS surface-potential solver diverged");
		}
		if (std::fabs(correction) < _tolerance) {
			double result = x * voltageNormalization;
			RequireFiniteResult(result, "surface potential");
			return result;
		}
	}
	throw std::runtime_error("AOS surface-potential solver did not converge");
}

double AOSFETCompactModel::CalculateDriftCurrent(double _vgs, double _vds) const {
	if (!std::isfinite(_vds) || _vds < 0) {
		throw std::invalid_argument("AOS drain voltage must be finite and nonnegative");
	}
	double sourceSurfacePotential = SurfacePotential(_vgs, 0.0);
	double drainSurfacePotential = SurfacePotential(_vgs, _vds);
	double sourceChargeTerm = _vgs - parameters.flatBandVoltage - sourceSurfacePotential;
	double drainChargeTerm = _vgs - parameters.flatBandVoltage - drainSurfacePotential;
	double diffusionTerm = 2 * thermalVoltage
			* (drainSurfacePotential - sourceSurfacePotential);
	double driftTerm = 0.5 * (drainChargeTerm * drainChargeTerm
			- sourceChargeTerm * sourceChargeTerm);
	double current = Mobility(_vgs) * (parameters.width / parameters.length)
			* parameters.oxideCapacitance * (diffusionTerm - driftTerm);
	RequireFiniteResult(current, "drift current");
	return std::max(current, 0.0);
}

double AOSFETCompactModel::CalculateLeakageCurrent(double _vgs, double _vds) const {
	RequireFinite(_vgs, "Vgs");
	if (!std::isfinite(_vds) || _vds < 0) {
		throw std::invalid_argument("AOS drain voltage must be finite and nonnegative");
	}
	double exponent = _vgs / (parameters.leakageSwingFactor * thermalVoltage);
	double saturationTerm = 1 - LimExp(-_vds / thermalVoltage);
	double current = parameters.leakageScale * leakagePrefactor * LimExp(exponent)
			* saturationTerm + parameters.currentFloor;
	RequireFiniteResult(current, "leakage current");
	if (current < 0) {
		throw std::runtime_error("AOS compact model produced negative leakage current");
	}
	return current;
}

double AOSFETCompactModel::CalculateDrainCurrent(double _vgs, double _vds) const {
	return _vgs < parameters.flatBandVoltage
			? CalculateLeakageCurrent(_vgs, _vds)
			: CalculateDriftCurrent(_vgs, _vds);
}

double AOSFETCompactModel::CalculateDrainCurrentExternal(double _vgs, double _vds) const {
	RequireFinite(_vgs, "Vgs");
	if (!std::isfinite(_vds) || _vds <= 0) {
		throw std::invalid_argument("AOS external-current drain voltage must be finite and positive");
	}
	double rsd = TotalContactResistanceOhm();
	if (rsd == 0) {
		return CalculateDrainCurrent(_vgs, _vds);
	}

	double rs = rsd / 2.0;
	double currentLimit = _vds / rsd;
	double high = std::min(std::max(CalculateDrainCurrent(_vgs, _vds), 1e-30), currentLimit);
	auto residual = [&](double current) {
		return CalculateDrainCurrent(_vgs - current * rs,
				std::max(_vds - current * rsd, 0.0)) - current;
	};
	double residualHigh = residual(high);
	for (int expansion = 0; residualHigh > 0 && high < currentLimit
			&& expansion < 80; ++expansion) {
		high = std::min(high * 2.0, currentLimit);
		residualHigh = residual(high);
	}
	if (!std::isfinite(residualHigh) || residualHigh > 0) {
		throw std::runtime_error("AOS contact-resistance solver failed to bracket a solution");
	}

	double low = 0.0;
	bool converged = false;
	for (int i = 0; i < 80; ++i) {
		double mid = 0.5 * (low + high);
		double residualMid = residual(mid);
		if (!std::isfinite(residualMid)) {
			throw std::runtime_error("AOS contact-resistance solver produced a non-finite residual");
		}
		if (residualMid > 0) {
			low = mid;
		} else {
			high = mid;
		}
		if (high - low <= std::max(1e-30, 1e-12 * high)) {
			converged = true;
			break;
		}
	}
	if (!converged)
		throw std::runtime_error("AOS contact-resistance solver did not converge");
	double current = 0.5 * (low + high);
	RequireFiniteResult(current, "external drain current");
	if (current <= 0) {
		throw std::runtime_error("AOS compact model produced nonpositive external drain current");
	}
	return current;
}

double AOSFETCompactModel::TotalContactResistanceOhm() const {
	if (parameters.totalContactResistanceKohmUm == 0) {
		return 0;
	}
	double resistance = parameters.totalContactResistanceKohmUm * 1e3
			/ (parameters.width * 1e6);
	RequireFiniteResult(resistance, "contact resistance");
	return resistance;
}

AOSFETCompactModel::Charges AOSFETCompactModel::CalculateCharges(
		double _vgs, double _vds) const {
	RequireFinite(_vgs, "Vgs");
	if (!std::isfinite(_vds) || _vds < 0) {
		throw std::invalid_argument("AOS drain voltage must be finite and nonnegative");
	}
	Charges charges = {0, 0};
	if (_vgs < parameters.flatBandVoltage) {
		return charges;
	}
	double sourceSurfacePotential = SurfacePotential(_vgs, 0.0);
	double drainSurfacePotential = SurfacePotential(_vgs, _vds);
	double sourceChargeTerm = _vgs - parameters.flatBandVoltage - sourceSurfacePotential;
	double drainChargeTerm = _vgs - parameters.flatBandVoltage - drainSurfacePotential;
	double chargeSum = sourceChargeTerm + drainChargeTerm;
	if (chargeSum < 1e-12) {
		return charges;
	}
	double gateNumerator = 2 * (sourceChargeTerm * sourceChargeTerm
			+ drainChargeTerm * drainChargeTerm
			+ sourceChargeTerm * drainChargeTerm);
	charges.gate = parameters.width * parameters.length * parameters.oxideCapacitance
			* gateNumerator / (3 * chargeSum);
	double drainNumerator = 2 * (2 * std::pow(sourceChargeTerm, 3)
			+ 4 * sourceChargeTerm * sourceChargeTerm * drainChargeTerm
			+ 6 * sourceChargeTerm * drainChargeTerm * drainChargeTerm
			+ 3 * std::pow(drainChargeTerm, 3));
	charges.drain = parameters.width * parameters.length * parameters.oxideCapacitance
			* drainNumerator / (15 * chargeSum * chargeSum);
	RequireFiniteResult(charges.gate, "gate charge");
	RequireFiniteResult(charges.drain, "drain charge");
	return charges;
}

AOSFETCompactModel::Capacitances AOSFETCompactModel::CalculateCapacitances(
		double _vgs, double _vds, double _delta) const {
	RequirePositive(_delta, "finite-difference voltage step");
	Charges baseCharges = CalculateCharges(_vgs, _vds);
	Charges drainStepCharges = CalculateCharges(_vgs, _vds + _delta);
	double intrinsicGateDrain = -(drainStepCharges.gate - baseCharges.gate) / _delta;
	Charges gateStepCharges = CalculateCharges(_vgs + _delta, _vds);
	double gateGate = (gateStepCharges.gate - baseCharges.gate) / _delta;
	Capacitances capacitances = {
		gateGate - intrinsicGateDrain + parameters.overlapCapacitance,
		intrinsicGateDrain + parameters.overlapCapacitance
	};
	if (_vgs < parameters.flatBandVoltage) {
		capacitances.gateSource = parameters.overlapCapacitance;
		capacitances.gateDrain = parameters.overlapCapacitance;
	}
	RequireFiniteResult(capacitances.gateSource, "gate-source capacitance");
	RequireFiniteResult(capacitances.gateDrain, "gate-drain capacitance");
	if (capacitances.gateSource < 0 || capacitances.gateDrain < 0) {
		throw std::runtime_error("AOS compact model produced negative capacitance");
	}
	return capacitances;
}

AOSDeviceOperatingPoint AOSFETCompactModel::EvaluateOperatingPoint(
		double _vgsOn, double _vgsOff, double _vds) const {
	RequireFinite(_vgsOn, "on-gate voltage");
	RequireFinite(_vgsOff, "off-gate voltage");
	if (_vgsOn <= _vgsOff) {
		throw std::invalid_argument("AOS on-gate voltage must exceed off-gate voltage");
	}
	if (!std::isfinite(_vds) || _vds <= 0) {
		throw std::invalid_argument("AOS operating-point Vds must be finite and positive");
	}

	AOSDeviceOperatingPoint point;
	point.Ion = CalculateDrainCurrentExternal(_vgsOn, _vds);
	point.Ioff = CalculateDrainCurrentExternal(_vgsOff, _vds);
	Capacitances capacitances = CalculateCapacitances(_vgsOn, _vds);
	point.Ron = _vds / point.Ion;
	point.Roff = _vds / point.Ioff;
	point.Cgate = capacitances.gateSource + capacitances.gateDrain;
	point.Cdrain = capacitances.gateDrain;
	RequireFiniteResult(point.Ion, "Ion");
	RequireFiniteResult(point.Ioff, "Ioff");
	RequireFiniteResult(point.Ron, "Ron");
	RequireFiniteResult(point.Roff, "Roff");
	RequireFiniteResult(point.Cgate, "gate capacitance");
	RequireFiniteResult(point.Cdrain, "drain capacitance");
	if (point.Ion <= point.Ioff || point.Ioff <= 0 || point.Ron <= 0
			|| point.Roff <= 0 || point.Cgate < 0 || point.Cdrain < 0) {
		throw std::runtime_error("AOS operating point is physically invalid (requires Ion > Ioff > 0, positive resistance, and nonnegative capacitance)");
	}
	return point;
}

double AOSFETCompactModel::LimExp(double _value) const {
	RequireFinite(_value, "exponential argument");
	return std::exp(std::min(std::max(_value, -50.0), 50.0));
}

double AOSFETCompactModel::GateNormalized(double _vgs) const {
	return (_vgs - parameters.flatBandVoltage) / voltageNormalization;
}

double AOSFETCompactModel::DrainNormalized(double _vds) const {
	return _vds / voltageNormalization;
}

double AOSFETCompactModel::InitialSurfacePotentialGuess(
		double _vgs, double _vds) const {
	double normalizedGate = GateNormalized(_vgs);
	double normalizedDrain = DrainNormalized(_vds);
	if (normalizedGate <= 0) {
		return normalizedGate;
	}
	double normalizedTrapCoefficient = trapCoefficient / voltageNormalization;
	double logArgument = std::max(normalizedGate / normalizedTrapCoefficient, 1e-12);
	double sqrtTerm = (normalizedGate + 1) * (normalizedGate + 1)
			+ 2 * normalizedDrain + 2 * std::log(logArgument);
	return sqrtTerm < 0 ? normalizedGate
			: normalizedGate * (std::sqrt(sqrtTerm) - normalizedGate - 1);
}
