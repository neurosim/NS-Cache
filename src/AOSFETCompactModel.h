/*******************************************************************************
* Copyright (c) 2025
* Georgia Institute of Technology
*
* This source code is part of NeuroSim (NS)-Cache.
*
* The AOS FET compact model is based on work by Jay Sonawane.
*******************************************************************************/

#ifndef AOSFETCOMPACTMODEL_H_
#define AOSFETCOMPACTMODEL_H_

struct AOSDeviceOperatingPoint {
	double Ion = 0;
	double Ioff = 0;
	double Ron = 0;
	double Roff = 0;
	double Cgate = 0;
	double Cdrain = 0;
};

class AOSFETCompactModel {
public:
	struct Parameters {
		double q;                 /* Elementary charge, Unit: C */
		double k;                 /* Boltzmann constant, Unit: J/K */
		double temperature;       /* Operating temperature, Unit: K */
		double tailTemperature;   /* DOS-tail characteristic temperature, Unit: K */
		double eps0;              /* Vacuum permittivity, Unit: F/m */
		double semiconductorPermittivity;
		double nu0T0;
		double flatBandVoltage;   /* Unit: V */
		double width;             /* Unit: m */
		double length;            /* Unit: m */
		double trapDensity;
		double fermiReference;
		double oxideCapacitance;  /* Unit: F/m^2 */
		double overlapCapacitance;/* Unit: F */

		double vrhThreshold;      /* Unit: V */
		double percolationThreshold; /* Unit: V */
		double vrhPrefactor;      /* Unit: cm^2/Vs */
		double percolationPrefactor; /* Unit: cm^2/Vs */
		double percolationExponent;
		bool useConstantMobility;
		double mobility;          /* Unit: m^2/Vs */
		double mobilityScale;

		double leakageSwingFactor;
		double leakageSurfacePotentialReference;
		double leakageScale;
		double currentFloor;      /* Unit: A */
		double totalContactResistanceKohmUm;
	};

	struct Charges {
		double gate;
		double drain;
	};

	struct Capacitances {
		double gateSource;
		double gateDrain;
	};

	AOSFETCompactModel();
	explicit AOSFETCompactModel(const Parameters &_parameters);

	void Initialize(const Parameters &_parameters);
	static Parameters DefaultParameters();

	double Mobility(double _vgs) const;
	double SurfacePotential(double _vgs, double _vch, int _maxIteration = 40,
			double _tolerance = 1e-8) const;
	double CalculateDriftCurrent(double _vgs, double _vds) const;
	double CalculateLeakageCurrent(double _vgs, double _vds) const;
	double CalculateDrainCurrent(double _vgs, double _vds) const;
	double CalculateDrainCurrentExternal(double _vgs, double _vds) const;
	double TotalContactResistanceOhm() const;
	Charges CalculateCharges(double _vgs, double _vds) const;
	Capacitances CalculateCapacitances(double _vgs, double _vds,
			double _delta = 1e-3) const;
	AOSDeviceOperatingPoint EvaluateOperatingPoint(double _vgsOn,
			double _vgsOff, double _vds) const;

	Parameters parameters;

private:
	void ValidateParameters() const;
	void UpdateDerivedParameters();
	double LimExp(double _value) const;
	double GateNormalized(double _vgs) const;
	double DrainNormalized(double _vds) const;
	double InitialSurfacePotentialGuess(double _vgs, double _vds) const;

	double mobilityVrhExponent;
	double voltageNormalization;
	double thermalVoltage;
	double trapCoefficient;
	double deepStateCoefficient;
	double leakagePrefactor;
};

#endif /* AOSFETCOMPACTMODEL_H_ */
