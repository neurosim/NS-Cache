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

// This file contains code from NVSim, (c) 2012-2013,  Pennsylvania State University 
//and Hewlett-Packard Company. See LICENSE_NVSim file in the top-level directory.
//No part of DESTINY Project, including this file, may be copied,
//modified, propagated, or distributed except according to the terms
//contained in the LICENSE file.


#ifndef FORMULA_H_
#define FORMULA_H_

#include "Technology.h"
#include "constant.h"
#include <math.h>

#define MAX(a,b) (((a)> (b))?(a):(b))
#define MIN(a,b) (((a)< (b))?(a):(b))

bool isPow2(int n);

/* calculate the gate capacitance */
double CalculateGateCap(double width, Technology tech);

double CalculateGateArea(
		int gateType, int numInput,
		double widthNMOS, double widthPMOS,
		double heightTransistorRegion, Technology tech,
		double *height, double *width);

/* calculate the capacitance of a gate */
void CalculateGateCapacitance(
		int gateType, int numInput,
		double widthNMOS, double widthPMOS,
		double heightTransistorRegion, Technology tech,
		double *capInput, double *capOutput);

// GAA special layout 
void CalculateGateCapacitance_GAA(
		int gateType, int numInput,
		double widthNMOS, double widthPMOS,
		double heightTransistorRegion, Technology tech,
		double *capInput, double *capOutput, double Gatefactor, double Ntuningfactor, double Ptuningfactor);

double CalculateDrainCap(
		double width, int type,
		double heightTransistorRegion, Technology tech);

/* calculate the capacitance of a FBRAM */
double CalculateFBRAMGateCap(double width, double thicknessFactor, Technology tech);

double CalculateFBRAMDrainCap(double width, Technology tech);

double CalculateGateLeakage(
		int gateType, int numInput,
		double widthNMOS, double widthPMOS,
		double temperature, Technology tech);

double CalculateOnResistance(double width, int type, double temperature, Technology tech);

double CalculateOffResistance(double width, int type, double temperature, Technology tech);

double CalculateTransconductance(double width, int type, Technology tech);

double horowitz(double tr, double beta, double rampInput, double *rampOutput);

double CalculateWireResistance_M0(
		double resistivity, double wireWidth, double wireThickness,
		double barrierThickness, double dishingThickness, double alphaScatter, bool neurosim_wiring, Technology tech);

double CalculateWireResistance_MX(
		double resistivity, double wireWidth, double wireThickness,
		double barrierThickness, double dishingThickness, double alphaScatter, bool neurosim_wiring, Technology tech);

double CalculateWireCapacitance(
		double permittivity, double wireWidth, double wireThickness, double wireSpacing,
		double ildThickness, double millarValue, double horizontalDielectric,
		double verticalDielectic, double fringeCap, bool neurosim_wiring);

double CalculateOnResistance_normal(double width, int type, double temperature, Technology tech);

void EnlargeSize(double *widthNMOS, double *widthPMOS, double heightTransistorRegion, Technology tech);

#endif /* FORMULA_H_ */
