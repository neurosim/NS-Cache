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
// This file contains code from CACTI-3DD, (c) 2012 Hewlett-Packard Development Company, L.P.
//See LICENSE_CACTI3DD file in the top-level directory.
//No part of DESTINY Project, including this file, may be copied,
//modified, propagated, or distributed except according to the terms
//contained in the LICENSE file.

#include "Technology.h"
#include "constant.h"
#include <math.h>

Technology::Technology() {
	// TODO Auto-generated constructor stub
	initialized = false;
    layerCount = 0;
}

Technology::~Technology() {
	// TODO Auto-generated destructor stub
}

void Technology::Initialize(int _featureSizeInNano, DeviceRoadmap _deviceRoadmap, InputParameter *inputParameter) {
	if (initialized)
		cout << "Warning: Already initialized!" << endl;

	double caplist [7] = {103.816,97.549,100.497,81.859,72.572, 79.74, 66.94}; // 69.369
	double currentlist [7] = {595.045, 599.237, 562.048, 578.494, 641.463, 526.868, 460.979}; //  556.448
	double currentlist_off [7] = {0.0001,0.000127, 0.000147, 0.000138, 0.000158, 0.0000733, 0.000169}; //0.000569
	double eff_res_mul [7] = {2.09, 2.09, 2.05, 2.10, 2.14, 1.98, 2.05};
	double gm [7] = {1415.34, 1803.50, 1785.37, 1820.90, 2018.04, 1968.85, 2401.75};
	double vth_list [7] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1}; // dummy values, since we don't need them
	double cap_draintotallist [7] = {2.499e-17, 2.668e-17, 2.224e-17, 2.076e-17, 1.791e-17, 1.543e-17, 1.409e-17};

	featureSizeInNano = _featureSizeInNano;
	featureSize = _featureSizeInNano * 1e-9;
	deviceRoadmap = _deviceRoadmap;

	if (_featureSizeInNano >= 180) { //TO-DO : only for test
		if (_deviceRoadmap == HP) {
			vdd = 1.5;
			vth = 300e-3;
			phyGateLength = 0.1e-6;
			capIdealGate = 8e-10;
			capFringe = 2.5e-10;
			capJunction = 1.00e-3;
			capOx = 1e-2;
			effectiveElectronMobility = 320e-4;
			effectiveHoleMobility = 80e-4;
			pnSizeRatio = 2.45;						/* from CACTI */
			effectiveResistanceMultiplier = 1.54;	/* from CACTI */
			currentOnNmos[0] = 750;
			currentOnNmos[10] = 750;
			currentOnNmos[20] = 750;
			currentOnNmos[30] = 750;
			currentOnNmos[40] = 750;
			currentOnNmos[50] = 750;
			currentOnNmos[60] = 750;
			currentOnNmos[70] = 750;
			currentOnNmos[80] = 750;
			currentOnNmos[90] = 750;
			currentOnNmos[100] = 750;
			currentOnPmos[0] = 350;
			currentOnPmos[10] = 350;
			currentOnPmos[20] = 350;
			currentOnPmos[30] = 350;
			currentOnPmos[40] = 350;
			currentOnPmos[50] = 350;
			currentOnPmos[60] = 350;
			currentOnPmos[70] = 350;
			currentOnPmos[80] = 350;
			currentOnPmos[90] = 350;
			currentOnPmos[100] = 350;
			currentOffNmos[0] = 8e-3;
			currentOffNmos[10] = 8e-3;
			currentOffNmos[20] = 8e-3;
			currentOffNmos[30] = 8e-3;
			currentOffNmos[40] = 8e-3;
			currentOffNmos[50] = 8e-3;
			currentOffNmos[60] = 8e-3;
			currentOffNmos[70] = 8e-3;
			currentOffNmos[80] = 8e-3;
			currentOffNmos[90] = 8e-3;
			currentOffNmos[100] = 8e-3;
			currentOffPmos[0] = 1.6e-2;
			currentOffPmos[10] = 1.6e-2;
			currentOffPmos[20] = 1.6e-2;
			currentOffPmos[30] = 1.6e-2;
			currentOffPmos[40] = 1.6e-2;
			currentOffPmos[50] = 1.6e-2;
			currentOffPmos[60] = 1.6e-2;
			currentOffPmos[70] = 1.6e-2;
			currentOffPmos[80] = 1.6e-2;
			currentOffPmos[90] = 1.6e-2;
			currentOffPmos[100] = 1.6e-2;
		} else if (_deviceRoadmap == LSTP) {
			vdd = 1.5;
			vth = 600e-3;
			phyGateLength = 0.16e-6;
			capIdealGate = 8e-10;
			capFringe = 2.5e-10;
			capJunction = 1.00e-3;
			capOx = 1e-2;
			effectiveElectronMobility = 320e-4;
			effectiveHoleMobility = 80e-4;
			pnSizeRatio = 2.45;						/* from CACTI */
			effectiveResistanceMultiplier = 1.54;	/* from CACTI */
			currentOnNmos[0] = 330;
			currentOnNmos[10] = 330;
			currentOnNmos[20] = 330;
			currentOnNmos[30] = 330;
			currentOnNmos[40] = 330;
			currentOnNmos[50] = 330;
			currentOnNmos[60] = 330;
			currentOnNmos[70] = 330;
			currentOnNmos[80] = 330;
			currentOnNmos[90] = 330;
			currentOnNmos[100] = 330;
			currentOnPmos[0] = 168;
			currentOnPmos[10] =  168;
			currentOnPmos[20] =  168;
			currentOnPmos[30] =  168;
			currentOnPmos[40] =  168;
			currentOnPmos[50] =  168;
			currentOnPmos[60] =  168;
			currentOnPmos[70] =  168;
			currentOnPmos[80] =  168;
			currentOnPmos[90] =  168;
			currentOnPmos[100] =  168;
			currentOffNmos[0] = 4.25e-6;
			currentOffNmos[10] = 4.25e-6;
			currentOffNmos[20] = 4.25e-6;
			currentOffNmos[30] = 4.25e-6;
			currentOffNmos[40] = 4.25e-6;
			currentOffNmos[50] = 4.25e-6;
			currentOffNmos[60] = 4.25e-6;
			currentOffNmos[70] = 4.25e-6;
			currentOffNmos[80] = 4.25e-6;
			currentOffNmos[90] = 4.25e-6;
			currentOffNmos[100] = 4.25e-6;
			currentOffPmos[0] = 8.5e-6;
			currentOffPmos[10] = 8.5e-6;
			currentOffPmos[20] = 8.5e-6;
			currentOffPmos[30] = 8.5e-6;
			currentOffPmos[40] = 8.5e-6;
			currentOffPmos[50] = 8.5e-6;
			currentOffPmos[60] = 8.5e-6;
			currentOffPmos[70] = 8.5e-6;
			currentOffPmos[80] = 8.5e-6;
			currentOffPmos[90] = 8.5e-6;
			currentOffPmos[100] = 8.5e-6;
		} else if (_deviceRoadmap == LOP) {
			vdd = 1.2;
			vth = 450e-3;
			phyGateLength = 0.135e-6;
			capIdealGate = 8e-10;
			capFringe = 2.5e-10;
			capJunction = 1.00e-3;
			capOx = 1e-2;
			effectiveElectronMobility = 330e-4;
			effectiveHoleMobility = 90e-4;
			pnSizeRatio = 2.45;						/* from CACTI */
			effectiveResistanceMultiplier = 1.54;	/* from CACTI */
			currentOnNmos[0] = 490;
			currentOnNmos[10] = 490;
			currentOnNmos[20] = 490;
			currentOnNmos[30] = 490;
			currentOnNmos[40] = 490;
			currentOnNmos[50] = 490;
			currentOnNmos[60] = 490;
			currentOnNmos[70] = 490;
			currentOnNmos[80] = 490;
			currentOnNmos[90] = 490;
			currentOnNmos[100] = 490;
			currentOnPmos[0] = 230;
			currentOnPmos[10] = 230;
			currentOnPmos[20] = 230;
			currentOnPmos[30] = 230;
			currentOnPmos[40] = 230;
			currentOnPmos[50] = 230;
			currentOnPmos[60] = 230;
			currentOnPmos[70] = 230;
			currentOnPmos[80] = 230;
			currentOnPmos[90] = 230;
			currentOnPmos[100] = 230;
			currentOffNmos[0] = 4e-4;
			currentOffNmos[10] = 4e-4;
			currentOffNmos[20] = 4e-4;
			currentOffNmos[30] = 4e-4;
			currentOffNmos[40] = 4e-4;
			currentOffNmos[50] = 4e-4;
			currentOffNmos[60] = 4e-4;
			currentOffNmos[70] = 4e-4;
			currentOffNmos[80] = 4e-4;
			currentOffNmos[90] = 4e-4;
			currentOffNmos[100] = 4e-4;
			currentOffPmos[0] = 8e-4;
			currentOffPmos[10] = 8e-4;
			currentOffPmos[20] = 8e-4;
			currentOffPmos[30] = 8e-4;
			currentOffPmos[40] = 8e-4;
			currentOffPmos[50] = 8e-4;
			currentOffPmos[60] = 8e-4;
			currentOffPmos[70] = 8e-4;
			currentOffPmos[80] = 8e-4;
			currentOffPmos[90] = 8e-4;
			currentOffPmos[100] = 8e-4;
        } else {
            cout << "Unknown device roadmap!" << endl;
            exit(1);
		}
	} else if (_featureSizeInNano >= 120) {	/* TO-DO: actually 100nm */
		if (_deviceRoadmap == HP) {
			/* MASTAR 5 - HP100.pro, HP100p.pro */
			vdd = 1.2;
			vth = 218.04e-3;
			phyGateLength = 0.0451e-6;
			capIdealGate = 7.41e-10;
			capFringe = 2.4e-10;
			capJunction = 1.00e-3;
			capOx = 1.64e-2;
			effectiveElectronMobility = 249.59e-4;
			effectiveHoleMobility = 59.52e-4;
			pnSizeRatio = 2.45;						/* from CACTI */
			effectiveResistanceMultiplier = 1.54;	/* from CACTI */
			currentOnNmos[0] = 960.9;
			currentOnNmos[10] = 947.9;
			currentOnNmos[20] = 935.1;
			currentOnNmos[30] = 922.5;
			currentOnNmos[40] = 910.0;
			currentOnNmos[50] = 897.7;
			currentOnNmos[60] = 885.5;
			currentOnNmos[70] = 873.6;
			currentOnNmos[80] = 861.8;
			currentOnNmos[90] = 850.1;
			currentOnNmos[100] = 838.6;
			currentOnPmos[0] = 578.4;
			currentOnPmos[10] = 567.8;
			currentOnPmos[20] = 557.5;
			currentOnPmos[30] = 547.4;
			currentOnPmos[40] = 537.5;
			currentOnPmos[50] = 527.8;
			currentOnPmos[60] = 518.3;
			currentOnPmos[70] = 509.1;
			currentOnPmos[80] = 500.0;
			currentOnPmos[90] = 491.1;
			currentOnPmos[100] = 482.5;
			currentOffNmos[0] = 1.90e-2;
			currentOffNmos[10] = 2.35e-2;
			currentOffNmos[20] = 2.86e-2;
			currentOffNmos[30] = 3.45e-2;
			currentOffNmos[40] = 4.12e-2;
			currentOffNmos[50] = 4.87e-2;
			currentOffNmos[60] = 5.71e-2;
			currentOffNmos[70] = 6.64e-2;
			currentOffNmos[80] = 7.67e-2;
			currentOffNmos[90] = 8.80e-2;
			currentOffNmos[100] = 1.00e-1;
			currentOffPmos[0] = 3.82e-2;
			currentOffPmos[10] = 3.84e-2;
			currentOffPmos[20] = 3.87e-2;
			currentOffPmos[30] = 3.90e-2;
			currentOffPmos[40] = 3.93e-2;
			currentOffPmos[50] = 3.97e-2;
			currentOffPmos[60] = 4.01e-2;
			currentOffPmos[70] = 4.05e-2;
			currentOffPmos[80] = 4.10e-2;
			currentOffPmos[90] = 4.16e-2;
			currentOffPmos[100] = 4.22e-2;
		} else if (_deviceRoadmap == LSTP) {
			/* MASTAR 5 - LSTP100.pro */
			vdd = 1.2;
			vth = 501.25e-3;
			phyGateLength = 0.075e-6;
			capIdealGate = 8.62e-10;
			capFringe = 2.5e-10;
			capJunction = 1.00e-3;
			capOx = 1.15e-2;
			effectiveElectronMobility = 284.97e-4;
			effectiveHoleMobility = 61.82e-4;
			pnSizeRatio = 2.45;						/* from CACTI */
			effectiveResistanceMultiplier = 1.54;	/* from CACTI */
			currentOnNmos[0] = 422.5;
			currentOnNmos[10] = 415.0;
			currentOnNmos[20] = 407.7;
			currentOnNmos[30] = 400.5;
			currentOnNmos[40] = 393.6;
			currentOnNmos[50] = 386.8;
			currentOnNmos[60] = 380.1;
			currentOnNmos[70] = 373.7;
			currentOnNmos[80] = 367.4;
			currentOnNmos[90] = 361.3;
			currentOnNmos[100] = 355.5;
			currentOnPmos[0] = 204.9;
			currentOnPmos[10] = 200.3;
			currentOnPmos[20] = 195.9;
			currentOnPmos[30] = 191.7;
			currentOnPmos[40] = 187.5;
			currentOnPmos[50] = 183.5;
			currentOnPmos[60] = 179.7;
			currentOnPmos[70] = 175.9;
			currentOnPmos[80] = 172.3;
			currentOnPmos[90] = 168.8;
			currentOnPmos[100] = 165.4;
			currentOffNmos[0] = 1.01e-5;
			currentOffNmos[10] = 1.04e-5;
			currentOffNmos[20] = 1.06e-5;
			currentOffNmos[30] = 1.09e-5;
			currentOffNmos[40] = 1.12e-5;
			currentOffNmos[50] = 1.16e-5;
			currentOffNmos[60] = 1.20e-5;
			currentOffNmos[70] = 1.24e-5;
			currentOffNmos[80] = 1.28e-5;
			currentOffNmos[90] = 1.32e-5;
			currentOffNmos[100] = 1.37e-5;
			currentOffPmos[0] = 2.21e-5;
			currentOffPmos[10] = 2.27e-5;
			currentOffPmos[20] = 2.33e-5;
			currentOffPmos[30] = 2.40e-5;
			currentOffPmos[40] = 2.47e-5;
			currentOffPmos[50] = 2.54e-5;
			currentOffPmos[60] = 2.62e-5;
			currentOffPmos[70] = 2.71e-5;
			currentOffPmos[80] = 2.80e-5;
			currentOffPmos[90] = 2.90e-5;
			currentOffPmos[100] = 3.01e-5;
		} else if (_deviceRoadmap == LOP) {
			/* MASTAR 5 - LOP100.pro */
			vdd = 1.0;
			vth = 312.60e-3;
			phyGateLength = 0.065e-6;
			capIdealGate = 6.34e-10;
			capFringe = 2.5e-10;
			capJunction = 1.00e-3;
			capOx = 1.44e-2;
			effectiveElectronMobility = 292.43e-4;
			effectiveHoleMobility = 64.53e-4;
			pnSizeRatio = 2.45;						/* from CACTI */
			effectiveResistanceMultiplier = 1.54;	/* from CACTI */
			currentOnNmos[0] = 531.4;
			currentOnNmos[10] = 522.6;
			currentOnNmos[20] = 514.0;
			currentOnNmos[30] = 505.5;
			currentOnNmos[40] = 497.3;
			currentOnNmos[50] = 489.2;
			currentOnNmos[60] = 481.3;
			currentOnNmos[70] = 473.6;
			currentOnNmos[80] = 466.1;
			currentOnNmos[90] = 458.8;
			currentOnNmos[100] = 451.6;
			currentOnPmos[0] = 278.5;
			currentOnPmos[10] = 272.5;
			currentOnPmos[20] = 266.8;
			currentOnPmos[30] = 261.2;
			currentOnPmos[40] = 255.8;
			currentOnPmos[50] = 250.5;
			currentOnPmos[60] = 245.4;
			currentOnPmos[70] = 240.4;
			currentOnPmos[80] = 235.6;
			currentOnPmos[90] = 231.0;
			currentOnPmos[100] = 226.4;
			currentOffNmos[0] = 9.69e-4;
			currentOffNmos[10] = 9.87e-4;
			currentOffNmos[20] = 1.01e-3;
			currentOffNmos[30] = 1.03e-3;
			currentOffNmos[40] = 1.05e-3;
			currentOffNmos[50] = 1.08e-3;
			currentOffNmos[60] = 1.10e-3;
			currentOffNmos[70] = 1.13e-3;
			currentOffNmos[80] = 1.16e-3;
			currentOffNmos[90] = 1.19e-3;
			currentOffNmos[100] = 1.23e-3;
			currentOffPmos[0] = 2.20e-3;
			currentOffPmos[10] = 2.25e-3;
			currentOffPmos[20] = 2.29e-3;
			currentOffPmos[30] = 2.34e-3;
			currentOffPmos[40] = 2.39e-3;
			currentOffPmos[50] = 2.45e-3;
			currentOffPmos[60] = 2.51e-3;
			currentOffPmos[70] = 2.57e-3;
			currentOffPmos[80] = 2.64e-3;
			currentOffPmos[90] = 2.72e-3;
			currentOffPmos[100] = 2.79e-3;
        } else {
            cout << "Unknown device roadmap!" << endl;
            exit(1);
		}
	} else if (_featureSizeInNano >= 90) {
		if (_deviceRoadmap == HP) {
			/* MASTAR 5 - HP90.pro */
			vdd = 1.2;
			vth = 197.95e-3;
			phyGateLength = 0.037e-6;
			capIdealGate = 6.38e-10;
			capFringe = 2.5e-10;
			capJunction = 1.00e-3;
			capOx = 1.73e-2;
			effectiveElectronMobility = 243.43e-4;
			effectiveHoleMobility = 58.32e-4;
			pnSizeRatio = 2.45;						/* from CACTI */
			effectiveResistanceMultiplier = 1.54;	/* from CACTI */
			currentOnNmos[0] = 1050.5;
			currentOnNmos[10] = 1037.0;
			currentOnNmos[20] = 1023.6;
			currentOnNmos[30] = 1010.3;
			currentOnNmos[40] = 997.2;
			currentOnNmos[50] = 984.2;
			currentOnNmos[60] = 971.4;
			currentOnNmos[70] = 958.8;
			currentOnNmos[80] = 946.3;
			currentOnNmos[90] = 933.9;
			currentOnNmos[100] = 921.7;
			currentOnPmos[0] = 638.7;
			currentOnPmos[10] = 627.5;
			currentOnPmos[20] = 616.5;
			currentOnPmos[30] = 605.8;
			currentOnPmos[40] = 595.2;
			currentOnPmos[50] = 584.9;
			currentOnPmos[60] = 574.7;
			currentOnPmos[70] = 564.8;
			currentOnPmos[80] = 555.1;
			currentOnPmos[90] = 545.5;
			currentOnPmos[100] = 536.2;
			currentOffNmos[0] = 1.90e-2 * 2.73;		/* correct from MASTAR */
			currentOffNmos[10] = 2.35e-2 * 2.73;	/* correct from MASTAR */
			currentOffNmos[20] = 2.86e-2 * 2.73;	/* correct from MASTAR */
			currentOffNmos[30] = 3.45e-2 * 2.73;	/* correct from MASTAR */
			currentOffNmos[40] = 4.12e-2 * 2.73;	/* correct from MASTAR */
			currentOffNmos[50] = 4.87e-2 * 2.73;	/* correct from MASTAR */
			currentOffNmos[60] = 5.71e-2 * 2.73;	/* correct from MASTAR */
			currentOffNmos[70] = 6.64e-2 * 2.73;	/* correct from MASTAR */
			currentOffNmos[80] = 7.67e-2 * 2.73;	/* correct from MASTAR */
			currentOffNmos[90] = 8.80e-2 * 2.73;	/* correct from MASTAR */
			currentOffNmos[100] = 1.00e-1 * 2.73;	/* correct from MASTAR */
			currentOffPmos[0] = 5.26e-2;
			currentOffPmos[10] = 5.26e-2;
			currentOffPmos[20] = 5.26e-2;
			currentOffPmos[30] = 5.27e-2;
			currentOffPmos[40] = 5.28e-2;
			currentOffPmos[50] = 5.29e-2;
			currentOffPmos[60] = 5.31e-2;
			currentOffPmos[70] = 5.34e-2;
			currentOffPmos[80] = 5.36e-2;
			currentOffPmos[90] = 5.40e-2;
			currentOffPmos[100] = 5.43e-2;
		} else if (_deviceRoadmap == LSTP) {
			/* MASTAR 5 - LSTP90.pro */
			vdd = 1.2;
			vth = 502.36e-3;
			phyGateLength = 0.065e-6;
			capIdealGate = 7.73e-10;
			capFringe = 2.4e-10;
			capJunction = 1.00e-3;
			capOx = 1.19e-2;
			effectiveElectronMobility = 277.94e-4;
			effectiveHoleMobility = 60.64e-4;
			pnSizeRatio = 2.44;						/* from CACTI */
			effectiveResistanceMultiplier = 1.92;	/* from CACTI */
			currentOnNmos[0] = 446.6;
			currentOnNmos[10] = 438.7;
			currentOnNmos[20] = 431.2;
			currentOnNmos[30] = 423.8;
			currentOnNmos[40] = 416.7;
			currentOnNmos[50] = 409.7;
			currentOnNmos[60] = 402.9;
			currentOnNmos[70] = 396.3;
			currentOnNmos[80] = 389.8;
			currentOnNmos[90] = 383.5;
			currentOnNmos[100] = 377.3;
			currentOnPmos[0] = 221.5;
			currentOnPmos[10] = 216.6;
			currentOnPmos[20] = 212.0;
			currentOnPmos[30] = 207.4;
			currentOnPmos[40] = 203.1;
			currentOnPmos[50] = 198.8;
			currentOnPmos[60] = 194.7;
			currentOnPmos[70] = 190.7;
			currentOnPmos[80] = 186.9;
			currentOnPmos[90] = 183.1;
			currentOnPmos[100] = 179.5;
			currentOffNmos[0] = 9.45e-6;
			currentOffNmos[10] = 9.67e-6;
			currentOffNmos[20] = 9.91e-5;
			currentOffNmos[30] = 1.02e-5;
			currentOffNmos[40] = 1.05e-5;
			currentOffNmos[50] = 1.08e-5;
			currentOffNmos[60] = 1.11e-5;
			currentOffNmos[70] = 1.14e-5;
			currentOffNmos[80] = 1.28e-5;
			currentOffNmos[90] = 1.32e-5;
			currentOffNmos[100] = 1.37e-5;
			currentOffPmos[0] = 2.05e-5;
			currentOffPmos[10] = 2.10e-5;
			currentOffPmos[20] = 2.15e-5;
			currentOffPmos[30] = 2.21e-5;
			currentOffPmos[40] = 2.27e-5;
			currentOffPmos[50] = 2.34e-5;
			currentOffPmos[60] = 2.41e-5;
			currentOffPmos[70] = 2.48e-5;
			currentOffPmos[80] = 2.56e-5;
			currentOffPmos[90] = 2.65e-5;
			currentOffPmos[100] = 2.74e-5;
		} else if (_deviceRoadmap == LOP) {
			/* MASTAR 5 - LOP90.pro */
			vdd = 0.9;
			vth = 264.54e-3;
			phyGateLength = 0.053e-6;
			capIdealGate = 7.95e-10;
			capFringe = 2.4e-10;
			capJunction = 1.00e-3;
			capOx = 1.50e-2;
			effectiveElectronMobility = 309.04e-4;
			effectiveHoleMobility = 67.88e-4;
			pnSizeRatio = 2.54;						/* from CACTI */
			effectiveResistanceMultiplier = 1.77;	/* from CACTI */
			currentOnNmos[0] = 534.5;
			currentOnNmos[10] = 525.7;
			currentOnNmos[20] = 517.0;
			currentOnNmos[30] = 508.5;
			currentOnNmos[40] = 500.2;
			currentOnNmos[50] = 492.1;
			currentOnNmos[60] = 484.1;
			currentOnNmos[70] = 476.3;
			currentOnNmos[80] = 468.7;
			currentOnNmos[90] = 461.2;
			currentOnNmos[100] = 453.9;
			currentOnPmos[0] = 294.2;
			currentOnPmos[10] = 287.8;
			currentOnPmos[20] = 281.7;
			currentOnPmos[30] = 275.7;
			currentOnPmos[40] = 269.9;
			currentOnPmos[50] = 264.2;
			currentOnPmos[60] = 258.7;
			currentOnPmos[70] = 253.4;
			currentOnPmos[80] = 248.2;
			currentOnPmos[90] = 243.2;
			currentOnPmos[100] = 238.3;
			currentOffNmos[0] = 2.74e-3;
			currentOffNmos[10] = 2.6e-3;
			currentOffNmos[20] = 2.79e-3;
			currentOffNmos[30] = 2.81e-3;
			currentOffNmos[40] = 2.84e-3;
			currentOffNmos[50] = 2.88e-3;
			currentOffNmos[60] = 2.91e-3;
			currentOffNmos[70] = 2.95e-3;
			currentOffNmos[80] = 2.99e-3;
			currentOffNmos[90] = 3.04e-3;
			currentOffNmos[100] =3.09e-3;
			currentOffPmos[0] = 6.51e-3;
			currentOffPmos[10] = 6.56e-3;
			currentOffPmos[20] = 6.61e-3;
			currentOffPmos[30] = 6.67e-3;
			currentOffPmos[40] = 6.74e-3;
			currentOffPmos[50] = 6.82e-3;
			currentOffPmos[60] = 6.91e-3;
			currentOffPmos[70] = 7.00e-3;
			currentOffPmos[80] = 7.10e-3;
			currentOffPmos[90] = 7.21e-3;
			currentOffPmos[100] = 7.33e-3;
		} else if (_deviceRoadmap == EDRAM) {
            vdd = 1.2; // V
            vpp = 1.6;
            vth = 454.5e-3; // V
            phyGateLength = 0.12e-6; // M
            capIdealGate = 1.47e-9;
            capFringe = 0.08e-9;
            capJunction = 1e-3;
            capOx = 1.22e-2; // fF
            effectiveElectronMobility = 323.95e-4;
            effectiveHoleMobility = 323.95e-4; // TODO: same as above
            pnSizeRatio = 1.95;
            effectiveResistanceMultiplier = 1.65;
            currentOnNmos[0] = 321.6;
            currentOnNmos[10] = 321.6;
            currentOnNmos[20] = 321.6;
            currentOnNmos[30] = 321.6;
            currentOnNmos[40] = 321.6;
            currentOnNmos[50] = 321.6;
            currentOnNmos[60] = 321.6;
            currentOnNmos[70] = 321.6;
            currentOnNmos[80] = 321.6;
            currentOnNmos[90] = 321.6;
            currentOnNmos[100] = 321.6;
            currentOnPmos[0] = 203.3;
            currentOnPmos[10] = 203.3;
            currentOnPmos[20] = 203.3;
            currentOnPmos[30] = 203.3;
            currentOnPmos[40] = 203.3;
            currentOnPmos[50] = 203.3;
            currentOnPmos[60] = 203.3;
            currentOnPmos[70] = 203.3;
            currentOnPmos[80] = 203.3;
            currentOnPmos[90] = 203.3;
            currentOnPmos[100] = 203.3;
            currentOffNmos[0] = 1.42e-5;
            currentOffNmos[10] = 2.25e-5;
            currentOffNmos[20] = 3.46e-5;
            currentOffNmos[30] = 5.18e-5;
            currentOffNmos[40] = 7.58e-5;
            currentOffNmos[50] = 1.08e-4;
            currentOffNmos[60] = 1.51e-4;
            currentOffNmos[70] = 2.02e-4;
            currentOffNmos[80] = 2.57e-4;
            currentOffNmos[90] = 3.14e-4;
            currentOffNmos[100] = 3.85e-4;
            currentOffPmos[0] = 1.42e-5;
            currentOffPmos[10] = 2.25e-5;
            currentOffPmos[20] = 3.46e-5;
            currentOffPmos[30] = 5.18e-5;
            currentOffPmos[40] = 7.58e-5;
            currentOffPmos[50] = 1.08e-4;
            currentOffPmos[60] = 1.51e-4;
            currentOffPmos[70] = 2.02e-4;
            currentOffPmos[80] = 2.57e-4;
            currentOffPmos[90] = 3.14e-4;
            currentOffPmos[100] = 3.85e-4;
        } else {
            cout << "Unknown device roadmap!" << endl;
            exit(1);
        }
	} else if (_featureSizeInNano >= 65) {
		if (_deviceRoadmap == HP) {
			/* MASTAR 5 - hp-bulk-2007.pro */
			vdd = 1.1;
			vth = 163.98e-3;
			phyGateLength = 0.025e-6;
			capIdealGate = 4.70e-10;
			capFringe = 2.4e-10;
			capJunction = 1.00e-3;
			capOx = 1.88e-2;
			effectiveElectronMobility = 445.74e-4;
			effectiveHoleMobility = 113.330e-4;
			pnSizeRatio = 2.41;						/* from CACTI */
			effectiveResistanceMultiplier = 1.50;	/* from CACTI */
			currentOnNmos[0] = 1211.4;
			currentOnNmos[10] = 1198.4;
			currentOnNmos[20] = 1185.4;
			currentOnNmos[30] = 1172.5;
			currentOnNmos[40] = 1156.9;
			currentOnNmos[50] = 1146.7;
			currentOnNmos[60] = 1133.6;
			currentOnNmos[70] = 1119.9;
			currentOnNmos[80] = 1104.3;
			currentOnNmos[90] = 1084.6;
			currentOnNmos[100] = 1059.0;
			currentOnPmos[0] = 888.7;
			currentOnPmos[10] = 875.8;
			currentOnPmos[20] = 861.7;
			currentOnPmos[30] = 848.5;
			currentOnPmos[40] = 835.4;
			currentOnPmos[50] = 822.6;
			currentOnPmos[60] = 809.9;
			currentOnPmos[70] = 797.3;
			currentOnPmos[80] = 784.8;
			currentOnPmos[90] = 772.2;
			currentOnPmos[100] = 759.4;
			currentOffNmos[0] = 3.43e-1;
			currentOffNmos[10] = 3.73e-1;
			currentOffNmos[20] = 4.03e-1;
			currentOffNmos[30] = 4.35e-1;
			currentOffNmos[40] = 4.66e-1;
			currentOffNmos[50] = 4.99e-1;
			currentOffNmos[60] = 5.31e-1;
			currentOffNmos[70] = 5.64e-1;
			currentOffNmos[80] = 5.96e-1;
			currentOffNmos[90] = 6.25e-1;
			currentOffNmos[100] = 6.51e-1;
			currentOffPmos[0] = 5.68e-1;
			currentOffPmos[10] = 6.07e-1;
			currentOffPmos[20] = 6.46e-1;
			currentOffPmos[30] = 6.86e-1;
			currentOffPmos[40] = 7.26e-1;
			currentOffPmos[50] = 7.66e-1;
			currentOffPmos[60] = 8.06e-1;
			currentOffPmos[70] = 8.46e-1;
			currentOffPmos[80] = 8.86e-1;
			currentOffPmos[90] = 9.26e-1;
			currentOffPmos[100] = 9.65e-1;
		} else if (_deviceRoadmap == LSTP) {
			/* MASTAR 5 - lstp-bulk-2007.pro */
			vdd = 1.1;
			vth = 563.92e-3;
			phyGateLength = 0.045e-6;
			capIdealGate = 6.17e-10;
			capFringe = 2.4e-10;
			capJunction = 1.00e-3;
			capOx = 1.37e-2;
			effectiveElectronMobility = 457.86e-4;
			effectiveHoleMobility = 102.64e-4;
			pnSizeRatio = 2.23;						/* from CACTI */
			effectiveResistanceMultiplier = 1.96;	/* from CACTI */
			currentOnNmos[0] = 465.4;
			currentOnNmos[10] = 458.5;
			currentOnNmos[20] = 451.8;
			currentOnNmos[30] = 445.1;
			currentOnNmos[40] = 438.4;
			currentOnNmos[50] = 431.6;
			currentOnNmos[60] = 423.9;
			currentOnNmos[70] = 414.2;
			currentOnNmos[80] = 400.6;
			currentOnNmos[90] = 383.5;
			currentOnNmos[100] = 367.2;
			currentOnPmos[0] = 234.2;
			currentOnPmos[10] = 229.7;
			currentOnPmos[20] = 225.3;
			currentOnPmos[30] = 221.0;
			currentOnPmos[40] = 216.8;
			currentOnPmos[50] = 212.7;
			currentOnPmos[60] = 208.8;
			currentOnPmos[70] = 204.8;
			currentOnPmos[80] = 200.7;
			currentOnPmos[90] = 196.6;
			currentOnPmos[100] = 192.6;
			currentOffNmos[0] = 3.03e-5;
			currentOffNmos[10] = 4.46e-5;
			currentOffNmos[20] = 6.43e-5;
			currentOffNmos[30] = 9.06e-5;
			currentOffNmos[40] = 1.25e-4;
			currentOffNmos[50] = 1.70e-4;
			currentOffNmos[60] = 2.25e-4;
			currentOffNmos[70] = 2.90e-4;
			currentOffNmos[80] = 3.61e-4;
			currentOffNmos[90] = 4.35e-4;
			currentOffNmos[100] = 5.20e-4;
			currentOffPmos[0] = 3.85e-5;
			currentOffPmos[10] = 5.64e-5;
			currentOffPmos[20] = 8.09e-5;
			currentOffPmos[30] = 1.14e-4;
			currentOffPmos[40] = 1.57e-4;
			currentOffPmos[50] = 2.12e-4;
			currentOffPmos[60] = 2.82e-4;
			currentOffPmos[70] = 3.70e-4;
			currentOffPmos[80] = 4.78e-4;
			currentOffPmos[90] = 6.09e-4;
			currentOffPmos[100] = 7.66e-4;
		} else if (_deviceRoadmap == LOP) {
			/* MASTAR 5 - lop-bulk-2007.pro */
			vdd = 0.8;
			vth = 323.75e-3;
			phyGateLength = 0.032e-6;
			capIdealGate = 6.01e-10;
			capFringe = 2.4e-10;
			capJunction = 1.00e-3;
			capOx = 1.88e-2;
			effectiveElectronMobility = 491.59e-4;
			effectiveHoleMobility = 110.95e-4;
			pnSizeRatio = 2.28;						/* from CACTI */
			effectiveResistanceMultiplier = 1.82;	/* from CACTI */
			currentOnNmos[0] = 562.9;
			currentOnNmos[10] = 555.2;
			currentOnNmos[20] = 547.5;
			currentOnNmos[30] = 539.8;
			currentOnNmos[40] = 532.2;
			currentOnNmos[50] = 524.5;
			currentOnNmos[60] = 516.1;
			currentOnNmos[70] = 505.7;
			currentOnNmos[80] = 491.1;
			currentOnNmos[90] = 471.7;
			currentOnNmos[100] = 451.6;
			currentOnPmos[0] = 329.5;
			currentOnPmos[10] = 323.3;
			currentOnPmos[20] = 317.2;
			currentOnPmos[30] = 311.2;
			currentOnPmos[40] = 305.4;
			currentOnPmos[50] = 299.8;
			currentOnPmos[60] = 294.2;
			currentOnPmos[70] = 288.7;
			currentOnPmos[80] = 283.2;
			currentOnPmos[90] = 277.5;
			currentOnPmos[100] = 271.8;
			currentOffNmos[0] = 9.08e-3;
			currentOffNmos[10] = 1.11e-2;
			currentOffNmos[20] = 1.35e-2;
			currentOffNmos[30] = 1.62e-2;
			currentOffNmos[40] = 1.92e-2;
			currentOffNmos[50] = 2.25e-2;
			currentOffNmos[60] = 2.62e-2;
			currentOffNmos[70] = 2.99e-2;
			currentOffNmos[80] = 3.35e-2;
			currentOffNmos[90] = 3.67e-2;
			currentOffNmos[100] = 3.98e-2;
			currentOffPmos[0] = 1.30e-2;
			currentOffPmos[10] = 1.57e-2;
			currentOffPmos[20] = 1.89e-2;
			currentOffPmos[30] = 2.24e-2;
			currentOffPmos[40] = 2.64e-2;
			currentOffPmos[50] = 3.08e-2;
			currentOffPmos[60] = 3.56e-2;
			currentOffPmos[70] = 1.09e-2;
			currentOffPmos[80] = 4.65e-2;
			currentOffPmos[90] = 5.26e-2;
			currentOffPmos[100] = 5.91e-2;
		} else if (_deviceRoadmap == EDRAM) {
            vdd = 1.2; // V
            vpp = 1.6;
            vth = 438.06e-3; // V
            phyGateLength = 0.12e-6; // M
            capIdealGate = 1.46e-9;
            capFringe = 0.08e-9;
            capJunction = 1e-3;
            capOx = 1.22e-2; // fF
            effectiveElectronMobility = 328.32e-4;
            effectiveHoleMobility = 328.32e-4; // TODO: same as above
            pnSizeRatio = 2.05;
            effectiveResistanceMultiplier = 1.65;
            currentOnNmos[0] = 399.8;
            currentOnNmos[10] = 399.8;
            currentOnNmos[20] = 399.8;
            currentOnNmos[30] = 399.8;
            currentOnNmos[40] = 399.8;
            currentOnNmos[50] = 399.8;
            currentOnNmos[60] = 399.8;
            currentOnNmos[70] = 399.8;
            currentOnNmos[80] = 399.8;
            currentOnNmos[90] = 399.8;
            currentOnNmos[100] = 399.8;
            currentOnPmos[0] = 243.4;
            currentOnPmos[10] = 243.4;
            currentOnPmos[20] = 243.4;
            currentOnPmos[30] = 243.4;
            currentOnPmos[40] = 243.4;
            currentOnPmos[50] = 243.4;
            currentOnPmos[60] = 243.4;
            currentOnPmos[70] = 243.4;
            currentOnPmos[80] = 243.4;
            currentOnPmos[90] = 243.4;
            currentOnPmos[100] = 243.4;
            currentOffNmos[0] = 2.23e-5;
            currentOffNmos[10] = 3.46e-5;
            currentOffNmos[20] = 5.24e-5;
            currentOffNmos[30] = 7.75e-5;
            currentOffNmos[40] = 1.12e-4;
            currentOffNmos[50] = 1.58e-4;
            currentOffNmos[60] = 2.18e-4;
            currentOffNmos[70] = 2.88e-4;
            currentOffNmos[80] = 3.63e-4;
            currentOffNmos[90] = 4.41e-4;
            currentOffNmos[100] = 5.36e-4;
            currentOffPmos[0] = 2.23e-5;
            currentOffPmos[10] = 3.46e-5;
            currentOffPmos[20] = 5.24e-5;
            currentOffPmos[30] = 7.75e-5;
            currentOffPmos[40] = 1.12e-4;
            currentOffPmos[50] = 1.58e-4;
            currentOffPmos[60] = 2.18e-4;
            currentOffPmos[70] = 2.88e-4;
            currentOffPmos[80] = 3.63e-4;
            currentOffPmos[90] = 4.41e-4;
            currentOffPmos[100] = 5.36e-4;
        } else {
            cout << "Unknown device roadmap!" << endl;
            exit(1);
        }
	} else if (_featureSizeInNano >= 45) {
		if (_deviceRoadmap == HP) {
			/* MASTAR 5 - hp-bulk-2010.pro */
			vdd = 1.0;
			vth = 126.79e-3;
			phyGateLength = 0.018e-6;
			capIdealGate = 6.78e-10;
			capFringe = 1.7e-10;
			capJunction = 1.00e-3;
			capOx = 3.77e-2;
			effectiveElectronMobility = 297.70e-4;
			effectiveHoleMobility = 95.27e-4;
			pnSizeRatio = 2.41;						/* from CACTI */
			effectiveResistanceMultiplier = 1.51;	/* from CACTI */
			currentOnNmos[0] = 1823.8;
			currentOnNmos[10] = 1808.2;
			currentOnNmos[20] = 1792.6;
			currentOnNmos[30] = 1777.0;
			currentOnNmos[40] = 1761.4;
			currentOnNmos[50] = 1745.8;
			currentOnNmos[60] = 1730.3;
			currentOnNmos[70] = 1714.7;
			currentOnNmos[80] = 1699.1;
			currentOnNmos[90] = 1683.2;
			currentOnNmos[100] = 1666.6;
			currentOnPmos[0] = 1632.2;
			currentOnPmos[10] = 1612.8;
			currentOnPmos[20] = 1593.6;
			currentOnPmos[30] = 1574.1;
			currentOnPmos[40] = 1554.7;
			currentOnPmos[50] = 1535.5;
			currentOnPmos[60] = 1516.4;
			currentOnPmos[70] = 1497.6;
			currentOnPmos[80] = 1478.8;
			currentOnPmos[90] = 1460.3;
			currentOnPmos[100] = 1441.8;
			/* NMOS off current, Unit: A/m, these values are calculated by Sheng Li offline */
			currentOffNmos[0] = 2.80e-1;
			currentOffNmos[10] = 3.28e-1;
			currentOffNmos[20] = 3.81e-1;
			currentOffNmos[30] = 4.39e-1;
			currentOffNmos[40] = 5.02e-1;
			currentOffNmos[50] = 5.69e-1;
			currentOffNmos[60] = 6.42e-1;
			currentOffNmos[70] = 7.20e-1;
			currentOffNmos[80] = 8.03e-1;
			currentOffNmos[90] = 8.91e-1;
			currentOffNmos[100] = 9.84e-1;
			/* PMOS off current, Unit: A/m. TO-DO: set PMOS = NMOS because MASTAR value is obviously wrong */
			currentOffPmos[0] = currentOffNmos[0];
			currentOffPmos[10] = currentOffNmos[10];
			currentOffPmos[20] = currentOffNmos[20];
			currentOffPmos[30] = currentOffNmos[30];
			currentOffPmos[40] = currentOffNmos[40];
			currentOffPmos[50] = currentOffNmos[50];
			currentOffPmos[60] = currentOffNmos[60];
			currentOffPmos[70] = currentOffNmos[70];
			currentOffPmos[80] = currentOffNmos[80];
			currentOffPmos[90] = currentOffNmos[90];
			currentOffPmos[100] = currentOffNmos[100];
		} else if (_deviceRoadmap == LSTP) {
			/* MASTAR 5 - lstp-bulk-2010.pro */
			vdd = 1.0;
			vth = 564.52e-3;
			phyGateLength = 0.028e-6;
			capIdealGate = 5.58e-10;
			capFringe = 2.1e-10;
			capJunction = 1.00e-3;
			capOx = 1.99e-2;
			effectiveElectronMobility = 456.14e-4;
			effectiveHoleMobility = 96.98e-4;
			pnSizeRatio = 2.23;						/* from CACTI */
			effectiveResistanceMultiplier = 1.99;	/* from CACTI */
			currentOnNmos[0] = 527.5;
			currentOnNmos[10] = 520.2;
			currentOnNmos[20] = 512.9;
			currentOnNmos[30] = 505.8;
			currentOnNmos[40] = 498.6;
			currentOnNmos[50] = 491.4;
			currentOnNmos[60] = 483.7;
			currentOnNmos[70] = 474.4;
			currentOnNmos[80] = 461.2;
			currentOnNmos[90] = 442.6;
			currentOnNmos[100] = 421.3;
			currentOnPmos[0] = 497.9;
			currentOnPmos[10] = 489.5;
			currentOnPmos[20] = 481.3;
			currentOnPmos[30] = 473.2;
			currentOnPmos[40] = 465.3;
			currentOnPmos[50] = 457.6;
			currentOnPmos[60] = 450.0;
			currentOnPmos[70] = 442.5;
			currentOnPmos[80] = 435.1;
			currentOnPmos[90] = 427.5;
			currentOnPmos[100] = 419.7;
			/* NMOS off current, Unit: A/m, these values are calculated by Sheng Li offline */
			currentOffNmos[0] = 1.01e-5;
			currentOffNmos[10] = 1.65e-5;
			currentOffNmos[20] = 2.62e-5;
			currentOffNmos[30] = 4.06e-5;
			currentOffNmos[40] = 6.12e-5;
			currentOffNmos[50] = 9.02e-5;
			currentOffNmos[60] = 1.30e-4;
			currentOffNmos[70] = 1.83e-4;
			currentOffNmos[80] = 2.51e-4;
			currentOffNmos[90] = 3.29e-4;
			currentOffNmos[100] = 4.10e-4;
			/* PMOS off current, Unit: A/m. TO-DO: set PMOS = NMOS because MASTAR value is obviously wrong */
			currentOffPmos[0] = currentOffNmos[0];
			currentOffPmos[10] = currentOffNmos[10];
			currentOffPmos[20] = currentOffNmos[20];
			currentOffPmos[30] = currentOffNmos[30];
			currentOffPmos[40] = currentOffNmos[40];
			currentOffPmos[50] = currentOffNmos[50];
			currentOffPmos[60] = currentOffNmos[60];
			currentOffPmos[70] = currentOffNmos[70];
			currentOffPmos[80] = currentOffNmos[80];
			currentOffPmos[90] = currentOffNmos[90];
			currentOffPmos[100] = currentOffNmos[100];
		} else if (_deviceRoadmap == LOP) {
			/* MASTAR 5 - lop-bulk-2010.pro */
			vdd = 0.7;
			vth = 288.94e-3;
			phyGateLength = 0.022e-6;
			capIdealGate = 6.13e-10;
			capFringe = 2.0e-10;
			capJunction = 1.00e-3;
			capOx = 2.79e-2;
			effectiveElectronMobility = 606.95e-4;
			effectiveHoleMobility = 124.60e-4;
			pnSizeRatio = 2.28;						/* from CACTI */
			effectiveResistanceMultiplier = 1.76;	/* from CACTI */
			currentOnNmos[0] = 682.1;
			currentOnNmos[10] = 672.3;
			currentOnNmos[20] = 662.5;
			currentOnNmos[30] = 652.8;
			currentOnNmos[40] = 643.0;
			currentOnNmos[50] = 632.8;
			currentOnNmos[60] = 620.9;
			currentOnNmos[70] = 605.0;
			currentOnNmos[80] = 583.6;
			currentOnNmos[90] = 561.0;
			currentOnNmos[100] = 542.7;
			currentOnPmos[0] = 772.4;
			currentOnPmos[10] = 759.6;
			currentOnPmos[20] = 746.9;
			currentOnPmos[30] = 734.4;
			currentOnPmos[40] = 722.1;
			currentOnPmos[50] = 710.0;
			currentOnPmos[60] = 698.1;
			currentOnPmos[70] = 686.3;
			currentOnPmos[80] = 674.4;
			currentOnPmos[90] = 662.3;
			currentOnPmos[100] = 650.2;
			/* NMOS off current, Unit: A/m, these values are calculated by Sheng Li offline */
			currentOffNmos[0] = 4.03e-3;
			currentOffNmos[10] = 5.02e-3;
			currentOffNmos[20] = 6.18e-3;
			currentOffNmos[30] = 7.51e-3;
			currentOffNmos[40] = 9.04e-3;
			currentOffNmos[50] = 1.08e-2;
			currentOffNmos[60] = 1.27e-2;
			currentOffNmos[70] = 1.47e-2;
			currentOffNmos[80] = 1.66e-2;
			currentOffNmos[90] = 1.84e-2;
			currentOffNmos[100] = 2.03e-2;
			/* PMOS off current, Unit: A/m. TO-DO: set PMOS = NMOS because MASTAR value is obviously wrong */
			currentOffPmos[0] = currentOffNmos[0];
			currentOffPmos[10] = currentOffNmos[10];
			currentOffPmos[20] = currentOffNmos[20];
			currentOffPmos[30] = currentOffNmos[30];
			currentOffPmos[40] = currentOffNmos[40];
			currentOffPmos[50] = currentOffNmos[50];
			currentOffPmos[60] = currentOffNmos[60];
			currentOffPmos[70] = currentOffNmos[70];
			currentOffPmos[80] = currentOffNmos[80];
			currentOffPmos[90] = currentOffNmos[90];
			currentOffPmos[100] = currentOffNmos[100];
		} else if (_deviceRoadmap == EDRAM) {
            vdd = 1.1; // V
            vpp = 1.5; 
            vth = 445.59e-3; // V
            phyGateLength = 0.078e-6; // M
            capIdealGate = 1.10e-9;
            capFringe = 0.08e-9;
            capJunction = 1e-3;
            capOx = 1.41e-2; // fF
            effectiveElectronMobility = 426.30e-4;
            effectiveHoleMobility = 426.30e-4; // TODO: same as above
            pnSizeRatio = 2.05;
            effectiveResistanceMultiplier = 1.65;
            currentOnNmos[0] = 456;
            currentOnNmos[10] = 456;
            currentOnNmos[20] = 456;
            currentOnNmos[30] = 456;
            currentOnNmos[40] = 456;
            currentOnNmos[50] = 456;
            currentOnNmos[60] = 456;
            currentOnNmos[70] = 456;
            currentOnNmos[80] = 456;
            currentOnNmos[90] = 456;
            currentOnNmos[100] = 456;
            currentOnPmos[0] = 228;
            currentOnPmos[10] = 228;
            currentOnPmos[20] = 228;
            currentOnPmos[30] = 228;
            currentOnPmos[40] = 228;
            currentOnPmos[50] = 228;
            currentOnPmos[60] = 228;
            currentOnPmos[70] = 228;
            currentOnPmos[80] = 228;
            currentOnPmos[90] = 228;
            currentOnPmos[100] = 228;
            currentOffNmos[0] = 2.54e-5;
            currentOffNmos[10] = 3.94e-5;
            currentOffNmos[20] = 5.95e-5;
            currentOffNmos[30] = 8.79e-5;
            currentOffNmos[40] = 1.27e-4;
            currentOffNmos[50] = 1.79e-4;
            currentOffNmos[60] = 2.47e-4;
            currentOffNmos[70] = 3.31e-4;
            currentOffNmos[80] = 4.26e-4;
            currentOffNmos[90] = 5.27e-4;
            currentOffNmos[100] = 6.46e-4;
            currentOffPmos[0] = 2.54e-5;
            currentOffPmos[10] = 3.94e-5;
            currentOffPmos[20] = 5.95e-5;
            currentOffPmos[30] = 8.79e-5;
            currentOffPmos[40] = 1.27e-4;
            currentOffPmos[50] = 1.79e-4;
            currentOffPmos[60] = 2.47e-4;
            currentOffPmos[70] = 3.31e-4;
            currentOffPmos[80] = 4.26e-4;
            currentOffPmos[90] = 5.27e-4;
            currentOffPmos[100] = 6.46e-4;
        } else {
            cout << "Unknown device roadmap!" << endl;
            exit(1);
        }
	} else if (_featureSizeInNano >= 32) {	/* TO-DO: actually 36nm */
		if (_deviceRoadmap == HP) {
			/* MASTAR 5 - hp-bulk-2012.pro */
			vdd = 0.9;
			vth = 131.72e-3;
			phyGateLength = 0.014e-6;
			capIdealGate = 6.42e-10;
			capFringe = 1.6e-10;
			capJunction = 1.00e-3;
			capOx = 4.59e-2;
			effectiveElectronMobility = 257.73e-4;
			effectiveHoleMobility = 89.92e-4;
			pnSizeRatio = 2.41;						/* from CACTI */
			effectiveResistanceMultiplier = 1.49;	/* from CACTI */
			currentOnNmos[0] = 1785.8;
			currentOnNmos[10] = 1771.8;
			currentOnNmos[20] = 1757.8;
			currentOnNmos[30] = 1743.8;
			currentOnNmos[40] = 1729.8;
			currentOnNmos[50] = 1715.7;
			currentOnNmos[60] = 1701.7;
			currentOnNmos[70] = 1687.6;
			currentOnNmos[80] = 1673.5;
			currentOnNmos[90] = 1659.4;
			currentOnNmos[100] = 1645.0;
			currentOnPmos[0] = 1713.5;
			currentOnPmos[10] = 1662.8;
			currentOnPmos[20] = 1620.1;
			currentOnPmos[30] = 1601.6;
			currentOnPmos[40] = 1583.3;
			currentOnPmos[50] = 1565.1;
			currentOnPmos[60] = 1547.1;
			currentOnPmos[70] = 1529.1;
			currentOnPmos[80] = 1511.3;
			currentOnPmos[90] = 1493.7;
			currentOnPmos[100] = 1476.1;
			currentOffNmos[0] = 8.34e-1;
			currentOffNmos[10] = 9.00e-1;
			currentOffNmos[20] = 9.68e-1;
			currentOffNmos[30] = 1.04;
			currentOffNmos[40] = 1.11;
			currentOffNmos[50] = 1.18;
			currentOffNmos[60] = 1.25;
			currentOffNmos[70] = 1.32;
			currentOffNmos[80] = 1.39;
			currentOffNmos[90] = 1.46;
			currentOffNmos[100] = 1.54;
			currentOffPmos[0] = currentOffNmos[0];	/* TO-DO: set PMOS = NMOS because MASTAR value is obviously wrong */
			currentOffPmos[10] = currentOffNmos[10];
			currentOffPmos[20] = currentOffNmos[20];
			currentOffPmos[30] = currentOffNmos[30];
			currentOffPmos[40] = currentOffNmos[40];
			currentOffPmos[50] = currentOffNmos[50];
			currentOffPmos[60] = currentOffNmos[60];
			currentOffPmos[70] = currentOffNmos[70];
			currentOffPmos[80] = currentOffNmos[80];
			currentOffPmos[90] = currentOffNmos[90];
			currentOffPmos[100] = currentOffNmos[100];
		} else if (_deviceRoadmap == LSTP) {
			/* MASTAR 5 - lstp-bulk-2012.pro */
			vdd = 1;
			vth = 581.81e-3;
			phyGateLength = 0.022e-6;
			capIdealGate = 5.02e-10;
			capFringe = 1.9e-10;
			capJunction = 1.00e-3;
			capOx = 2.19e-2;
			effectiveElectronMobility = 395.20e-4;
			effectiveHoleMobility = 88.67e-4;
			pnSizeRatio = 2.23;						/* from CACTI */
			effectiveResistanceMultiplier = 1.99;	/* from CACTI */
			currentOnNmos[0] = 560.0;
			currentOnNmos[10] = 553.0;
			currentOnNmos[20] = 546.1;
			currentOnNmos[30] = 539.3;
			currentOnNmos[40] = 532.5;
			currentOnNmos[50] = 525.8;
			currentOnNmos[60] = 518.9;
			currentOnNmos[70] = 511.5;
			currentOnNmos[80] = 502.3;
			currentOnNmos[90] = 489.2;
			currentOnNmos[100] = 469.7;
			currentOnPmos[0] = 549.6;
			currentOnPmos[10] = 541.1;
			currentOnPmos[20] = 532.8;
			currentOnPmos[30] = 524.6;
			currentOnPmos[40] = 516.5;
			currentOnPmos[50] = 508.7;
			currentOnPmos[60] = 500.9;
			currentOnPmos[70] = 493.3;
			currentOnPmos[80] = 485.8;
			currentOnPmos[90] = 478.3;
			currentOnPmos[100] = 470.7;
			currentOffNmos[0] = 3.02e-5;
			currentOffNmos[10] = 4.51e-5;
			currentOffNmos[20] = 6.57e-5;
			currentOffNmos[30] = 9.35e-5;
			currentOffNmos[40] = 1.31e-4;
			currentOffNmos[50] = 1.79e-4;
			currentOffNmos[60] = 2.41e-4;
			currentOffNmos[70] = 3.19e-4;
			currentOffNmos[80] = 4.15e-4;
			currentOffNmos[90] = 5.29e-4;
			currentOffNmos[100] = 6.58e-4;
			currentOffPmos[0] = currentOffNmos[0];	/* TO-DO: set PMOS = NMOS because MASTAR value is obviously wrong */
			currentOffPmos[10] = currentOffNmos[10];
			currentOffPmos[20] = currentOffNmos[20];
			currentOffPmos[30] = currentOffNmos[30];
			currentOffPmos[40] = currentOffNmos[40];
			currentOffPmos[50] = currentOffNmos[50];
			currentOffPmos[60] = currentOffNmos[60];
			currentOffPmos[70] = currentOffNmos[70];
			currentOffPmos[80] = currentOffNmos[80];
			currentOffPmos[90] = currentOffNmos[90];
			currentOffPmos[100] = currentOffNmos[100];
		} else if (_deviceRoadmap == LOP) {
			/* MASTAR 5 - lop-bulk-2012.pro */
			vdd = 0.7;
			vth = 278.52e-3;
			phyGateLength = 0.018e-6;
			capIdealGate = 5.54e-10;
			capFringe = 2.0e-10;
			capJunction = 1.00e-3;
			capOx = 3.08e-2;
			effectiveElectronMobility = 581.62e-4;
			effectiveHoleMobility = 120.30e-4;
			pnSizeRatio = 2.28;						/* from CACTI */
			effectiveResistanceMultiplier = 1.73;	/* from CACTI */
			currentOnNmos[0] = 760.3;
			currentOnNmos[10] = 750.4;
			currentOnNmos[20] = 740.5;
			currentOnNmos[30] = 730.7;
			currentOnNmos[40] = 720.8;
			currentOnNmos[50] = 710.9;
			currentOnNmos[60] = 700.3;
			currentOnNmos[70] = 687.6;
			currentOnNmos[80] = 670.5;
			currentOnNmos[90] = 647.4;
			currentOnNmos[100] = 623.6;
			currentOnPmos[0] = 878.6;  //TO-DO currentOnPmos even larger than currentOnNmos ?
			currentOnPmos[10] = 865.1;
			currentOnPmos[20] = 851.8;
			currentOnPmos[30] = 838.7;
			currentOnPmos[40] = 825.7;
			currentOnPmos[50] = 813.0;
			currentOnPmos[60] = 800.3;
			currentOnPmos[70] = 787.9;
			currentOnPmos[80] = 775.5;
			currentOnPmos[90] = 763.0;
			currentOnPmos[100] = 750.3;
			currentOffNmos[0] = 3.57e-2;
			currentOffNmos[10] = 4.21e-2;
			currentOffNmos[20] = 4.91e-2;
			currentOffNmos[30] = 5.68e-2;
			currentOffNmos[40] = 6.51e-2;
			currentOffNmos[50] = 7.42e-2;
			currentOffNmos[60] = 8.43e-2;
			currentOffNmos[70] = 9.57e-2;
			currentOffNmos[80] = 1.10e-1;
			currentOffNmos[90] = 1.28e-1;
			currentOffNmos[100] = 1.48e-1;
			currentOffPmos[0] = currentOffNmos[0];	/* TO-DO: set PMOS = NMOS because MASTAR value is obviously wrong */
			currentOffPmos[10] = currentOffNmos[10];
			currentOffPmos[20] = currentOffNmos[20];
			currentOffPmos[30] = currentOffNmos[30];
			currentOffPmos[40] = currentOffNmos[40];
			currentOffPmos[50] = currentOffNmos[50];
			currentOffPmos[60] = currentOffNmos[60];
			currentOffPmos[70] = currentOffNmos[70];
			currentOffPmos[80] = currentOffNmos[80];
			currentOffPmos[90] = currentOffNmos[90];
			currentOffPmos[100] = currentOffNmos[100];
		} else if (_deviceRoadmap == EDRAM) {
            vdd = 1.0; // V
            vpp = 1.5;
            vth = 441.29e-3; // V
            phyGateLength = 0.056e-6; // M
            capIdealGate = 7.45e-10;
            capFringe = 0.053e-9;
            capJunction = 1e-3;
            capOx = 1.48e-2; // fF
            effectiveElectronMobility = 408.12e-4;
            effectiveHoleMobility = 408.12e-4; // TODO: same as above
            pnSizeRatio = 2.05;
            effectiveResistanceMultiplier = 1.65;
            currentOnNmos[0] = 1055.4;
            currentOnNmos[10] = 1055.4;
            currentOnNmos[20] = 1055.4;
            currentOnNmos[30] = 1055.4;
            currentOnNmos[40] = 1055.4;
            currentOnNmos[50] = 1055.4;
            currentOnNmos[60] = 1055.4;
            currentOnNmos[70] = 1055.4;
            currentOnNmos[80] = 1055.4;
            currentOnNmos[90] = 1055.4;
            currentOnNmos[100] = 1055.4;
            currentOnPmos[0] = 527.7;
            currentOnPmos[10] = 527.7;
            currentOnPmos[20] = 527.7;
            currentOnPmos[30] = 527.7;
            currentOnPmos[40] = 527.7;
            currentOnPmos[50] = 527.7;
            currentOnPmos[60] = 527.7;
            currentOnPmos[70] = 527.7;
            currentOnPmos[80] = 527.7;
            currentOnPmos[90] = 527.7;
            currentOnPmos[100] = 527.7;
            currentOffNmos[0] = 3.57e-5;
            currentOffNmos[10] = 5.51e-5;
            currentOffNmos[20] = 8.27e-5;
            currentOffNmos[30] = 1.21e-4;
            currentOffNmos[40] = 1.74e-4;
            currentOffNmos[50] = 2.45e-4;
            currentOffNmos[60] = 3.38e-4;
            currentOffNmos[70] = 4.53e-4;
            currentOffNmos[80] = 5.87e-4;
            currentOffNmos[90] = 7.29e-4;
            currentOffNmos[100] = 8.87e-4;
            currentOffPmos[0] = 3.57e-5;
            currentOffPmos[10] = 5.51e-5;
            currentOffPmos[20] = 8.27e-5;
            currentOffPmos[30] = 1.21e-4;
            currentOffPmos[40] = 1.74e-4;
            currentOffPmos[50] = 2.45e-4;
            currentOffPmos[60] = 3.38e-4;
            currentOffPmos[70] = 4.53e-4;
            currentOffPmos[80] = 5.87e-4;
            currentOffPmos[90] = 7.29e-4;
            currentOffPmos[100] = 8.87e-4;
        } else {
            cout << "Unknown device roadmap!" << endl;
            exit(1);
        }
	} else if (_featureSizeInNano >= 22) {
		if (_deviceRoadmap == HP) {
			/* MASTAR 5 - hp-soi-2015.pro */ /* TO-DO: actually 25nm */
			vdd = 0.9;
			vth = 128.72e-3;
			phyGateLength = 0.010e-6;
			capIdealGate = 3.83e-10;
			capFringe = 1.6e-10;
			capJunction = 0;
			capOx = 3.83e-2;
			effectiveElectronMobility = 397.26e-4;
			effectiveHoleMobility = 83.60e-4;
			pnSizeRatio = 2;						/* from CACTI */
			effectiveResistanceMultiplier = 1.45;	/* from CACTI */
			currentOnNmos[0] = 2029.9;
			currentOnNmos[10] = 2009.8;
			currentOnNmos[20] = 1989.6;
			currentOnNmos[30] = 1969.6;
			currentOnNmos[40] = 1949.8;
			currentOnNmos[50] = 1930.7;
			currentOnNmos[60] = 1910.5;
			currentOnNmos[70] = 1891.0;
			currentOnNmos[80] = 1871.7;
			currentOnNmos[90] = 1852.5;
			currentOnNmos[100] = 1834.4;
			currentOnPmos[0] = currentOnNmos[0] / 2; /* TO-DO: MASTER values are absolutely wrong so that CACTI method is temporarily used here */
			currentOnPmos[10] = currentOnNmos[0] / 2;
			currentOnPmos[20] = currentOnNmos[10] / 2;
			currentOnPmos[30] = currentOnNmos[20] / 2;
			currentOnPmos[40] = currentOnNmos[30] / 2;
			currentOnPmos[50] = currentOnNmos[40] / 2;
			currentOnPmos[60] = currentOnNmos[50] / 2;
			currentOnPmos[70] = currentOnNmos[60] / 2;
			currentOnPmos[80] = currentOnNmos[70] / 2;
			currentOnPmos[90] = currentOnNmos[80] / 2;
			currentOnPmos[100] = currentOnNmos[90] / 2;
			currentOffNmos[0] = 1.52e-7 * 3.93e6; /* TO-DO: MASTER values are absolutely wrong so that data are translated from 32nm */
			currentOffNmos[10] = 1.55e-7 * 3.93e6;
			currentOffNmos[20] = 1.59e-7 * 3.93e6;
			currentOffNmos[30] = 1.68e-7 * 3.93e6;
			currentOffNmos[40] = 1.90e-7 * 3.93e6;
			currentOffNmos[50] = 2.69e-7 * 3.93e6;
			currentOffNmos[60] = 5.32e-7 * 3.93e6;
			currentOffNmos[70] = 1.02e-6 * 3.93e6;
			currentOffNmos[80] = 1.62e-6 * 3.93e6;
			currentOffNmos[90] = 2.73e-6 * 3.93e6;
			currentOffNmos[100] = 6.1e-6 * 3.93e6;
			currentOffPmos[0] = currentOffNmos[0];	/* TO-DO: set PMOS = NMOS because MASTAR value is obviously wrong */
			currentOffPmos[10] = currentOffNmos[10];
			currentOffPmos[20] = currentOffNmos[20];
			currentOffPmos[30] = currentOffNmos[30];
			currentOffPmos[40] = currentOffNmos[40];
			currentOffPmos[50] = currentOffNmos[50];
			currentOffPmos[60] = currentOffNmos[60];
			currentOffPmos[70] = currentOffNmos[70];
			currentOffPmos[80] = currentOffNmos[80];
			currentOffPmos[90] = currentOffNmos[90];
			currentOffPmos[100] = currentOffNmos[100];
		} else if (_deviceRoadmap == LSTP) {
			/* MASTAR 5 - lstp-bulk-2016.pro */
			vdd = 0.8;
			vth = 445.71e-3;
			phyGateLength = 0.016e-6;
			capIdealGate = 4.25e-10;
			capFringe = 2e-10;
			capJunction = 0;
			capOx = 2.65e-2;
			effectiveElectronMobility = 731.29e-4;
			effectiveHoleMobility = 111.22e-4;
			pnSizeRatio = 2.23;						/* from CACTI */
			effectiveResistanceMultiplier = 1.99;	/* from CACTI */
			currentOnNmos[0] = 745.5;
			currentOnNmos[10] = 735.2;
			currentOnNmos[20] = 725.1;
			currentOnNmos[30] = 715.2;
			currentOnNmos[40] = 705.4;
			currentOnNmos[50] = 695.7;
			currentOnNmos[60] = 686.2;
			currentOnNmos[70] = 676.9;
			currentOnNmos[80] = 667.7;
			currentOnNmos[90] = 658.7;
			currentOnNmos[100] = 649.8;
			currentOnPmos[0] = currentOnNmos[0] / 2; /* TO-DO: MASTER values are absolutely wrong so that CACTI method is temporarily used here */
			currentOnPmos[10] = currentOnNmos[0] / 2;
			currentOnPmos[20] = currentOnNmos[10] / 2;
			currentOnPmos[30] = currentOnNmos[20] / 2;
			currentOnPmos[40] = currentOnNmos[30] / 2;
			currentOnPmos[50] = currentOnNmos[40] / 2;
			currentOnPmos[60] = currentOnNmos[50] / 2;
			currentOnPmos[70] = currentOnNmos[60] / 2;
			currentOnPmos[80] = currentOnNmos[70] / 2;
			currentOnPmos[90] = currentOnNmos[80] / 2;
			currentOnPmos[100] = currentOnNmos[90] / 2;
			currentOffNmos[0] = 3.02e-5 / 1.86; /* TO-DO: MASTER values are absolutely wrong so that data are translated from 32nm */
			currentOffNmos[10] = 4.51e-5 / 1.86;
			currentOffNmos[20] = 6.57e-5 / 1.86;
			currentOffNmos[30] = 9.35e-5 / 1.86;
			currentOffNmos[40] = 1.31e-4 / 1.86;
			currentOffNmos[50] = 1.79e-4 / 1.86;
			currentOffNmos[60] = 2.41e-4 / 1.86;
			currentOffNmos[70] = 3.19e-4 / 1.86;
			currentOffNmos[80] = 4.15e-4 / 1.86;
			currentOffNmos[90] = 5.29e-4 / 1.86;
			currentOffNmos[100] = 6.58e-4 / 1.86;
			currentOffPmos[0] = currentOffNmos[0];	/* TO-DO: set PMOS = NMOS because MASTAR value is obviously wrong */
			currentOffPmos[10] = currentOffNmos[10];
			currentOffPmos[20] = currentOffNmos[20];
			currentOffPmos[30] = currentOffNmos[30];
			currentOffPmos[40] = currentOffNmos[40];
			currentOffPmos[50] = currentOffNmos[50];
			currentOffPmos[60] = currentOffNmos[60];
			currentOffPmos[70] = currentOffNmos[70];
			currentOffPmos[80] = currentOffNmos[80];
			currentOffPmos[90] = currentOffNmos[90];
			currentOffPmos[100] = currentOffNmos[100];
		} else if (_deviceRoadmap == LOP) {
			/* MASTAR 5 - lop-bulk-2016.pro */
			vdd = 0.5;
			vth = 217.39e-3;
			phyGateLength = 0.011e-6;
			capIdealGate = 3.45e-10;
			capFringe = 1.7e-10;
			capJunction = 0;
			capOx = 3.14e-2;
			effectiveElectronMobility = 747.37e-4;
			effectiveHoleMobility = 118.35e-4;
			pnSizeRatio = 2.28;						/* from CACTI */
			effectiveResistanceMultiplier = 1.73;	/* from CACTI */
			currentOnNmos[0] = 716.1;
			currentOnNmos[10] = 704.3;
			currentOnNmos[20] = 692.6;
			currentOnNmos[30] = 681.2;
			currentOnNmos[40] = 669.9;
			currentOnNmos[50] = 658.8;
			currentOnNmos[60] = 647.9;
			currentOnNmos[70] = 637.1;
			currentOnNmos[80] = 626.5;
			currentOnNmos[90] = 616.0;
			currentOnNmos[100] = 605.7;
			currentOnPmos[0] = currentOnNmos[0] / 2; /* TO-DO: MASTER values are absolutely wrong so that CACTI method is temporarily used here */
			currentOnPmos[10] = currentOnNmos[0] / 2;
			currentOnPmos[20] = currentOnNmos[10] / 2;
			currentOnPmos[30] = currentOnNmos[20] / 2;
			currentOnPmos[40] = currentOnNmos[30] / 2;
			currentOnPmos[50] = currentOnNmos[40] / 2;
			currentOnPmos[60] = currentOnNmos[50] / 2;
			currentOnPmos[70] = currentOnNmos[60] / 2;
			currentOnPmos[80] = currentOnNmos[70] / 2;
			currentOnPmos[90] = currentOnNmos[80] / 2;
			currentOnPmos[100] = currentOnNmos[90] / 2;
			currentOffNmos[0] = 3.57e-2 / 1.7;
			currentOffNmos[10] = 4.21e-2 / 1.7;
			currentOffNmos[20] = 4.91e-2 / 1.7;
			currentOffNmos[30] = 5.68e-2 / 1.7;
			currentOffNmos[40] = 6.51e-2 / 1.7;
			currentOffNmos[50] = 7.42e-2 / 1.7;
			currentOffNmos[60] = 8.43e-2 / 1.7;
			currentOffNmos[70] = 9.57e-2 / 1.7;
			currentOffNmos[80] = 1.10e-1 / 1.7;
			currentOffNmos[90] = 1.28e-1 / 1.7;
			currentOffNmos[100] = 1.48e-1 / 1.7;
			currentOffPmos[0] = currentOffNmos[0];	/* TO-DO: set PMOS = NMOS because MASTAR value is obviously wrong */
			currentOffPmos[10] = currentOffNmos[10];
			currentOffPmos[20] = currentOffNmos[20];
			currentOffPmos[30] = currentOffNmos[30];
			currentOffPmos[40] = currentOffNmos[40];
			currentOffPmos[50] = currentOffNmos[50];
			currentOffPmos[60] = currentOffNmos[60];
			currentOffPmos[70] = currentOffNmos[70];
			currentOffPmos[80] = currentOffNmos[80];
			currentOffPmos[90] = currentOffNmos[90];
			currentOffPmos[100] = currentOffNmos[100];
		} else if (_deviceRoadmap == EDRAM) {
            /* Projected based on the other technology nodes. */
            vdd = 0.9; // V
            vpp = 1.4;
            vth = 436.835e-3; // V
            phyGateLength = 0.010e-6; // M
            capIdealGate = 5.22e-10;
            capFringe = 0.008e-9;
            capJunction = 1e-3;
            capOx = 1.58e-2; // fF
            effectiveElectronMobility = 459.295e-4;
            effectiveHoleMobility = 459.295e-4; // TODO: same as above
            pnSizeRatio = 2.10;
            effectiveResistanceMultiplier = 1.65;
            currentOnNmos[0] = 1122.6;
            currentOnNmos[10] = 1122.6;
            currentOnNmos[20] = 1122.6;
            currentOnNmos[30] = 1122.6;
            currentOnNmos[40] = 1122.6;
            currentOnNmos[50] = 1122.6;
            currentOnNmos[60] = 1122.6;
            currentOnNmos[70] = 1122.6;
            currentOnNmos[80] = 1122.6;
            currentOnNmos[90] = 1122.6;
            currentOnNmos[100] = 1122.6;
            currentOnPmos[0] = 540.05;
            currentOnPmos[10] = 540.05;
            currentOnPmos[20] = 540.05;
            currentOnPmos[30] = 540.05;
            currentOnPmos[40] = 540.05;
            currentOnPmos[50] = 540.05;
            currentOnPmos[60] = 540.05;
            currentOnPmos[70] = 540.05;
            currentOnPmos[80] = 540.05;
            currentOnPmos[90] = 540.05;
            currentOnPmos[100] = 540.05;
            currentOffNmos[0] = 4.66e-5;
            currentOffNmos[10] = 6.36e-5;
            currentOffNmos[20] = 9.52e-5;
            currentOffNmos[30] = 1.39e-4;
            currentOffNmos[40] = 2.00e-4;
            currentOffNmos[50] = 2.81e-4;
            currentOffNmos[60] = 3.86e-4;
            currentOffNmos[70] = 5.18e-4;
            currentOffNmos[80] = 6.72e-4;
            currentOffNmos[90] = 8.36e-4;
            currentOffNmos[100] = 1.02e-3;
            currentOffPmos[0] = 4.66e-5;
            currentOffPmos[10] = 6.36e-5;
            currentOffPmos[20] = 9.52e-5;
            currentOffPmos[30] = 1.39e-4;
            currentOffPmos[40] = 2.00e-4;
            currentOffPmos[50] = 2.81e-4;
            currentOffPmos[60] = 3.86e-4;
            currentOffPmos[70] = 5.18e-4;
            currentOffPmos[80] = 6.72e-4;
            currentOffPmos[90] = 8.36e-4;
            currentOffPmos[100] = 1.03e-3;
        } else {
            cout << "Unknown device roadmap!" << endl;
            exit(1);
        }
	} else if (featureSizeInNano >= 14) {
			if (deviceRoadmap == HP) {
				
				// 1.4 update
				cout<<"HP for 14 nm and beyond is not supported"<<endl;
				exit(-1);

			} else {
				// 1.4 update : device specifications follow IRDS 2016
				vdd = 0.8;
				vth = vth_list[0];
				heightFin = 4.2e-8;
				widthFin = 8.0e-9;
				PitchFin = 4.8e-8;

				// 1.4 update
				max_fin_num =4;
				effective_width=widthFin+heightFin*2; // 9.2e-8

				phyGateLength = 2.6e-8; // 1.4 update : changed to 2.6e-8 following IRDS 2016
				capIdealGate = caplist[0] * 1E-18 / (effective_width);
				cap_draintotal = cap_draintotallist[0] / (effective_width);
				capFringe = 0;
				effectiveResistanceMultiplier = eff_res_mul[0];	/* from CACTI */
				current_gmNmos= gm[0];
				current_gmPmos= gm[0];	
				gm_oncurrent = gm[0];  // gm at on current

				// REF CACTI PARAMS
				//vdd = 1;									V
				//vth = 581.81e-3;							V
				//phyGateLength = 0.022e-6;					V
				//capIdealGate = 5.02e-10;					V
				//capFringe = 1.9e-10;						V
				//capJunction = 1.00e-3;					X - New Methods Introduced
				//capOx = 2.19e-2;							X - New Methods Introduced
				//effectiveElectronMobility = 395.20e-4;	X - New Methods Introduced
				//effectiveHoleMobility = 88.67e-4;			X - New Methods Introduced
				//pnSizeRatio = 2.23;						V
				//effectiveResistanceMultiplier = 1.99;		V

				/* NeuroSim 1.4 Only Conducts TCAD Tuning for Room Temp (300K) On Current. Restrict to Array 14nm -> 1nm in V.1.0 */
				currentOnNmos[0]  = currentlist[0];
				currentOnNmos[10] = currentlist[0];
				currentOnNmos[20] = currentlist[0];
				currentOnNmos[30] = currentlist[0];
				currentOnNmos[40] = currentlist[0];
				currentOnNmos[50] = currentlist[0];
				currentOnNmos[60] = currentlist[0];
				currentOnNmos[70] = currentlist[0];
				currentOnNmos[80] = currentlist[0];
				currentOnNmos[90] = currentlist[0];
				currentOnNmos[100] = currentlist[0];
				currentOnPmos[0]  = currentOnNmos[0];
				currentOnPmos[10] = currentOnNmos[0];
				currentOnPmos[20] = currentOnNmos[0];
				currentOnPmos[30] = currentOnNmos[0];
				currentOnPmos[40] = currentOnNmos[0];
				currentOnPmos[50] = currentOnNmos[0];
				currentOnPmos[60] = currentOnNmos[0];
				currentOnPmos[70] = currentOnNmos[0];
				currentOnPmos[80] = currentOnNmos[0];
				currentOnPmos[90] = currentOnNmos[0];
				currentOnPmos[100] = currentOnNmos[0];
				currentOffNmos[0]  = currentlist_off[0]; /* Use off current adjustment from NSv1.5*/
				currentOffNmos[10] = 184.4553e-6;
				currentOffNmos[20] = 328.7707e-6;
				currentOffNmos[30] = 566.8658e-6;
				currentOffNmos[40] = 948.1816e-6;
				currentOffNmos[50] = 1.5425e-3;
				currentOffNmos[60] = 2.4460e-3;
				currentOffNmos[70] = 3.7885e-3;
				currentOffNmos[80] = 5.7416e-3;
				currentOffNmos[90] = 8.5281e-3;
				currentOffNmos[100] =1.24327e-2;;
				currentOffPmos[0]  = 102.3333e-6;
				currentOffPmos[10] = 203.4774e-6;
				currentOffPmos[20] = 389.0187e-6;
				currentOffPmos[30] = 717.5912e-6;
				currentOffPmos[40] = 1.2810e-3;
				currentOffPmos[50] = 2.2192e-3;
				currentOffPmos[60] = 3.7395e-3;
				currentOffPmos[70] = 6.1428e-3;
				currentOffPmos[80] = 9.8554e-3;
				currentOffPmos[90] = 1.54702e-2;
				currentOffPmos[100] =2.37959e-2;
				pnSizeRatio = (int)(currentOnNmos[0]/currentOnPmos[0]);
			}
		} else if (featureSizeInNano >= 10) {
			if (deviceRoadmap == HP) {

				// 1.4 update
				cout<<"HP for 14 nm and beyond is not supported"<<endl;
				exit(-1);

	  		} else {

				// 1.4 update : device specifications follow IRDS 2017
				vdd = 0.75;
				vth = vth_list[1];
				heightFin = 4.5e-8;	
				widthFin = 8.0e-9;	
				PitchFin = 3.6e-8;			

				// 1.4 update 
				max_fin_num =3;
				effective_width=widthFin+heightFin*2; // 9.8e-8

				phyGateLength = 2.2e-8;	
				capIdealGate = caplist[1] * 1E-18 / (effective_width);
				cap_draintotal = cap_draintotallist[1]/ (effective_width);
				capFringe = 0;
				effectiveResistanceMultiplier = eff_res_mul[1];	/* from CACTI */
				current_gmNmos= gm[1];
				current_gmPmos= gm[1];			
				gm_oncurrent = gm[1];  // gm at on current

				/* NeuroSim 1.4 Only Conducts TCAD Tuning for Room Temp (300K) On Current. Restrict to Array 14nm -> 1nm in V.1.0 */
				currentOnNmos[0]  = currentlist[1];
				currentOnNmos[10] = currentlist[1];
				currentOnNmos[20] = currentlist[1];
				currentOnNmos[30] = currentlist[1];
				currentOnNmos[40] = currentlist[1];
				currentOnNmos[50] = currentlist[1];
				currentOnNmos[60] = currentlist[1];
				currentOnNmos[70] = currentlist[1];
				currentOnNmos[80] = currentlist[1];
				currentOnNmos[90] = currentlist[1];
				currentOnNmos[100] =currentlist[1];
				currentOnPmos[0]  = currentOnNmos[0];  
				currentOnPmos[10] = currentOnNmos[0];
				currentOnPmos[20] = currentOnNmos[0];
				currentOnPmos[30] = currentOnNmos[0];
				currentOnPmos[40] = currentOnNmos[0];
				currentOnPmos[50] = currentOnNmos[0];
				currentOnPmos[60] = currentOnNmos[0];
				currentOnPmos[70] = currentOnNmos[0];
				currentOnPmos[80] = currentOnNmos[0];
				currentOnPmos[90] = currentOnNmos[0];
				currentOnPmos[100] =currentOnNmos[0];
				currentOffNmos[0]  = currentlist_off[1]; /* Use off current adjustment from NSv1.5*/
				currentOffNmos[10] = 184.4892e-6;
				currentOffNmos[20] = 329.1615e-6;
				currentOffNmos[30] = 568.0731e-6;
				currentOffNmos[40] = 951.0401e-6;
				currentOffNmos[50] = 1.5484e-3;
				currentOffNmos[60] = 2.4574e-3;
				currentOffNmos[70] = 3.8090e-3;
				currentOffNmos[80] = 5.7767e-3;
				currentOffNmos[90] = 8.5862e-3;
				currentOffNmos[100] =1.2525e-2;
				currentOffPmos[0]  = 100.5839e-6;
				currentOffPmos[10] = 200.2609e-6;
				currentOffPmos[20] = 383.3239e-6;
				currentOffPmos[30] = 707.8499e-6;
				currentOffPmos[40] = 1.2649e-3;
				currentOffPmos[50] = 2.1932e-3;
				currentOffPmos[60] = 3.6987e-3;
				currentOffPmos[70] = 6.0804e-3;
				currentOffPmos[80] = 9.7622e-3;
				currentOffPmos[90] = 1.53340e-2;
				currentOffPmos[100] =2.36007e-2;
				pnSizeRatio = (int)(currentOnNmos[0]/currentOnPmos[0]);
			}
		} else if (featureSizeInNano >= 7) {
			if (deviceRoadmap == HP) {

				// 1.4 update
				cout<<"HP for 14 nm and beyond is not supported"<<endl;
				exit(-1);

			} else {

				// 1.4 update: based on IRDS 2017
				vdd = 0.7;
				vth = vth_list[2];
				heightFin = 5.0e-8;
				widthFin = 7e-9;
				PitchFin = 3.0e-8;			

				// 1.4 update
				max_fin_num = 2;
				effective_width=107e-9;	

				phyGateLength = 2.2e-8;
				capIdealGate = caplist[2] * 1E-18 / (effective_width);//8.49489e-10;
				cap_draintotal = cap_draintotallist[2]/ (effective_width);
				capFringe = 0;
				effectiveResistanceMultiplier = eff_res_mul[2];	/* from CACTI */
				current_gmNmos= gm[2];
				current_gmPmos= gm[2];
				gm_oncurrent = gm[2];  // gm at on current

				/* NeuroSim 1.4 Only Conducts TCAD Tuning for Room Temp (300K) On Current. Restrict to Array 14nm -> 1nm in V.1.0 */
				currentOnNmos[0]  = currentlist[2];
				currentOnNmos[10] = currentlist[2]; 
				currentOnNmos[20] = currentlist[2]; 
				currentOnNmos[30] = currentlist[2]; 
				currentOnNmos[40] = currentlist[2]; 
				currentOnNmos[50] = currentlist[2]; 
				currentOnNmos[60] = currentlist[2]; 
				currentOnNmos[70] = currentlist[2]; 
				currentOnNmos[80] = currentlist[2]; 
				currentOnNmos[90] = currentlist[2];
				currentOnNmos[100]= currentlist[2]; 
				currentOnPmos[0]  = currentOnNmos[0];  
				currentOnPmos[10] = currentOnNmos[0];  
				currentOnPmos[20] = currentOnNmos[0];  
				currentOnPmos[30] = currentOnNmos[0];  
				currentOnPmos[40] = currentOnNmos[0];  
				currentOnPmos[50] = currentOnNmos[0];  
				currentOnPmos[60] = currentOnNmos[0];  
				currentOnPmos[70] = currentOnNmos[0];  
				currentOnPmos[80] = currentOnNmos[0];  
				currentOnPmos[90] = currentOnNmos[0];  
				currentOnPmos[100] =currentOnNmos[0];  
				currentOffNmos[0]  = currentlist_off[2]; /* Use off current adjustment from NSv1.5*/
				currentOffNmos[10] = 1.85E-04;
				currentOffNmos[20] = 3.32E-04;
				currentOffNmos[30] = 5.74E-04;
				currentOffNmos[40] = 9.62E-04;
				currentOffNmos[50] = 1.5695e-3;
				currentOffNmos[60] = 2.4953e-3;
				currentOffNmos[70] = 3.8744e-3 ;
				currentOffNmos[80] = 5.8858e-3 ;
				currentOffNmos[90] = 8.7624e-3;
				currentOffNmos[100] =1.28025e-2;
				currentOffPmos[0]  = 100.9536e-6;
				currentOffPmos[10] = 201.3937e-6;
				currentOffPmos[20] = 386.2086e-6;
				currentOffPmos[30] = 714.4288e-6;
				currentOffPmos[40] = 1.2788e-3;
				currentOffPmos[50] = 2.2207e-3;
				currentOffPmos[60] = 3.7509e-3;
				currentOffPmos[70] = 6.1750e-3;
				currentOffPmos[80] = 9.9278e-3;
				currentOffPmos[90] = 1.56146e-2;
				currentOffPmos[100] =2.40633e-2;
				pnSizeRatio = (int)(currentOnNmos[0]/currentOnPmos[0]);
			}
		} 

		// 1.4 update: technology extension beyond 7 nm 

		/* Technology update beyond 7 nm */ 
		else if (featureSizeInNano >= 5) {
			if (deviceRoadmap == HP) {

				// 1.4 update
				cout<<"HP for 14 nm and beyond is not supported"<<endl;
				exit(-1);

			} else {

				// 1.4 update: IRDS 2021
				vdd = 0.7;
				vth = vth_list[3];

				widthFin=6.0e-9; 
				PitchFin=28.0e-9;	
				phyGateLength = 2.0e-8;

				// 1.4 update: height is not needed as long as effective width is specified
				effective_width = 106.0*1e-9;	
				max_fin_num =2;		

				capIdealGate = caplist[3] * 1E-18 / (effective_width );
				cap_draintotal = cap_draintotallist[3]/ (effective_width);
				capFringe = 0;

				effectiveResistanceMultiplier = eff_res_mul[3];	/* from CACTI */
				current_gmNmos= gm[3];
				current_gmPmos= gm[3];
				gm_oncurrent = gm[3];  // gm at on current

				/* NeuroSim 1.4 Only Conducts TCAD Tuning for Room Temp (300K) On Current. Restrict to Array 14nm -> 1nm in V.1.0 */
				currentOnNmos[0]  = currentlist[3];
				currentOnNmos[10] = currentlist[3]; 
				currentOnNmos[20] = currentlist[3]; 
				currentOnNmos[30] = currentlist[3]; 
				currentOnNmos[40] = currentlist[3]; 
				currentOnNmos[50] = currentlist[3]; 
				currentOnNmos[60] = currentlist[3]; 
				currentOnNmos[70] = currentlist[3]; 
				currentOnNmos[80] = currentlist[3]; 
				currentOnNmos[90] = currentlist[3];
				currentOnNmos[100]= currentlist[3]; 
				currentOnPmos[0]  = currentOnNmos[0]; 
				currentOnPmos[10] = currentOnNmos[0]; 
				currentOnPmos[20] = currentOnNmos[0]; 
				currentOnPmos[30] = currentOnNmos[0]; 
				currentOnPmos[40] = currentOnNmos[0]; 
				currentOnPmos[50] = currentOnNmos[0]; 
				currentOnPmos[60] = currentOnNmos[0]; 
				currentOnPmos[70] = currentOnNmos[0]; 
				currentOnPmos[80] = currentOnNmos[0]; 
				currentOnPmos[90] = currentOnNmos[0]; 
				currentOnPmos[100] =currentOnNmos[0]; 
				currentOffNmos[0]  = currentlist_off[3]; /* Use off current adjustment from NSv1.5*/
				currentOffNmos[10] = 1.85E-04;
				currentOffNmos[20] = 3.32E-04;
				currentOffNmos[30] = 5.74E-04;
				currentOffNmos[40] = 9.62E-04;
				currentOffNmos[50] = 1.5695e-3;
				currentOffNmos[60] = 2.4953e-3;
				currentOffNmos[70] = 3.8744e-3 ;
				currentOffNmos[80] = 5.8858e-3 ;
				currentOffNmos[90] = 8.7624e-3;
				currentOffNmos[100] =1.28025e-2;
				currentOffPmos[0]  = 100.9536e-6;
				currentOffPmos[10] = 201.3937e-6;
				currentOffPmos[20] = 386.2086e-6;
				currentOffPmos[30] = 714.4288e-6;
				currentOffPmos[40] = 1.2788e-3;
				currentOffPmos[50] = 2.2207e-3;
				currentOffPmos[60] = 3.7509e-3;
				currentOffPmos[70] = 6.1750e-3;
				currentOffPmos[80] = 9.9278e-3;
				currentOffPmos[90] = 1.56146e-2;
				currentOffPmos[100] =2.40633e-2;
				pnSizeRatio = (int)(currentOnNmos[0]/currentOnPmos[0]);
			}
		} else if (featureSizeInNano >= 3) {
			if (deviceRoadmap == HP) {

				// 1.4 update
				cout<<"HP for 14 nm and beyond is not supported"<<endl;
				exit(-1);

			} else {

				// 1.4 update: IRDS 2022
				vdd = 0.7;
				vth = vth_list[4];
				widthFin=5.0e-9;  	
				PitchFin=24.0e-9;	
				phyGateLength = 1.8e-8;

				// 1.4 update: height is not needed as long as effective width is specified
				effective_width = 101.0*1e-9;
				max_fin_num =2;	
				
				capIdealGate = caplist[4] * 1E-18 / (effective_width);   //6.44E-10; //8.91E-10;
				cap_draintotal = cap_draintotallist[4]/ (effective_width);
				capFringe = 0;

				effectiveResistanceMultiplier = eff_res_mul[4];	/* from CACTI */
				current_gmNmos= gm[4];
				current_gmPmos= gm[4];	
				gm_oncurrent = gm[4];  // gm at on current

				/* NeuroSim 1.4 Only Conducts TCAD Tuning for Room Temp (300K) On Current. Restrict to Array 14nm -> 1nm in V.1.0 */
				currentOnNmos[0]  = currentlist[4];
				currentOnNmos[10] = currentlist[4]; 
				currentOnNmos[20] = currentlist[4]; 
				currentOnNmos[30] = currentlist[4]; 
				currentOnNmos[40] = currentlist[4]; 
				currentOnNmos[50] = currentlist[4]; 
				currentOnNmos[60] = currentlist[4]; 
				currentOnNmos[70] = currentlist[4]; 
				currentOnNmos[80] = currentlist[4]; 
				currentOnNmos[90] = currentlist[4];
				currentOnNmos[100]= currentlist[4]; 
				currentOnPmos[0]  = currentOnNmos[0]; 
				currentOnPmos[10] = currentOnNmos[0]; 
				currentOnPmos[20] = currentOnNmos[0]; 
				currentOnPmos[30] = currentOnNmos[0]; 
				currentOnPmos[40] = currentOnNmos[0]; 
				currentOnPmos[50] = currentOnNmos[0]; 
				currentOnPmos[60] = currentOnNmos[0]; 
				currentOnPmos[70] = currentOnNmos[0]; 
				currentOnPmos[80] = currentOnNmos[0]; 
				currentOnPmos[90] = currentOnNmos[0]; 
				currentOnPmos[100] =currentOnNmos[0]; 
				currentOffNmos[0]  = currentlist_off[4]; /* Use off current adjustment from NSv1.5*/
				currentOffNmos[10] = 1.85E-04;
				currentOffNmos[20] = 3.32E-04;
				currentOffNmos[30] = 5.74E-04;
				currentOffNmos[40] = 9.62E-04;
				currentOffNmos[50] = 1.5695e-3;
				currentOffNmos[60] = 2.4953e-3;
				currentOffNmos[70] = 3.8744e-3 ;
				currentOffNmos[80] = 5.8858e-3 ;
				currentOffNmos[90] = 8.7624e-3;
				currentOffNmos[100] =1.28025e-2;
				currentOffPmos[0]  = 100.9536e-6;
				currentOffPmos[10] = 201.3937e-6;
				currentOffPmos[20] = 386.2086e-6;
				currentOffPmos[30] = 714.4288e-6;
				currentOffPmos[40] = 1.2788e-3;
				currentOffPmos[50] = 2.2207e-3;
				currentOffPmos[60] = 3.7509e-3;
				currentOffPmos[70] = 6.1750e-3;
				currentOffPmos[80] = 9.9278e-3;
				currentOffPmos[90] = 1.56146e-2;
				currentOffPmos[100] =2.40633e-2;
				pnSizeRatio = (int)(currentOnNmos[0]/currentOnPmos[0]);
			}
		} 
		else if (featureSizeInNano >= 2) {
			if (deviceRoadmap == HP) {

				// 1.4 update
				cout<<"HP for 14 nm and beyond is not supported"<<endl;
				exit(-1);

			} else { // 2 nm

				// 1.4 update: IRDS 2022
				vdd = 0.65;
				vth = vth_list[5];
				PitchFin= 26e-9;
				phyGateLength = 1.4e-8;

				// 1.4 update: GAA-specific parameters
				max_fin_per_GAA=1;
				max_sheet_num=3;
				thickness_sheet=6*1e-9;
				width_sheet=15*1e-9;	

				widthFin=width_sheet; // for drain height calculation 	
				effective_width=(thickness_sheet+width_sheet)*2;

				capIdealGate = caplist[5] * 1E-18 /  (effective_width*max_sheet_num) ; 
				cap_draintotal = cap_draintotallist[5]/ (effective_width);
				capFringe = 0;

				effectiveResistanceMultiplier = eff_res_mul[5];	/* from CACTI */
				current_gmNmos= gm[5];
				current_gmPmos= gm[5];	
				gm_oncurrent = gm[5];  // gm at on current

				/* NeuroSim 1.4 Only Conducts TCAD Tuning for Room Temp (300K) On Current. Restrict to Array 14nm -> 1nm in V.1.0 */
				currentOnNmos[0]  = currentlist[5];
				currentOnNmos[10] = currentlist[5]; 
				currentOnNmos[20] = currentlist[5]; 
				currentOnNmos[30] = currentlist[5]; 
				currentOnNmos[40] = currentlist[5]; 
				currentOnNmos[50] = currentlist[5]; 
				currentOnNmos[60] = currentlist[5]; 
				currentOnNmos[70] = currentlist[5]; 
				currentOnNmos[80] = currentlist[5]; 
				currentOnNmos[90] = currentlist[5];
				currentOnNmos[100]= currentlist[5]; 
				currentOnPmos[0]  = currentOnNmos[0]; 
				currentOnPmos[10] = currentOnNmos[0];
				currentOnPmos[20] = currentOnNmos[0];
				currentOnPmos[30] = currentOnNmos[0];
				currentOnPmos[40] = currentOnNmos[0];
				currentOnPmos[50] = currentOnNmos[0];
				currentOnPmos[60] = currentOnNmos[0];
				currentOnPmos[70] = currentOnNmos[0];
				currentOnPmos[80] = currentOnNmos[0];
				currentOnPmos[90] = currentOnNmos[0];
				currentOnPmos[100] =currentOnNmos[0];
				currentOffNmos[0]  = currentlist_off[5]; /* Use off current adjustment from NSv1.5*/
				currentOffNmos[10] = 1.85E-04;
				currentOffNmos[20] = 3.32E-04;
				currentOffNmos[30] = 5.74E-04;
				currentOffNmos[40] = 9.62E-04;
				currentOffNmos[50] = 1.5695e-3;
				currentOffNmos[60] = 2.4953e-3;
				currentOffNmos[70] = 3.8744e-3 ;
				currentOffNmos[80] = 5.8858e-3 ;
				currentOffNmos[90] = 8.7624e-3;
				currentOffNmos[100] =1.28025e-2;
				currentOffPmos[0]  = 100.9536e-6;
				currentOffPmos[10] = 201.3937e-6;
				currentOffPmos[20] = 386.2086e-6;
				currentOffPmos[30] = 714.4288e-6;
				currentOffPmos[40] = 1.2788e-3;
				currentOffPmos[50] = 2.2207e-3;
				currentOffPmos[60] = 3.7509e-3;
				currentOffPmos[70] = 6.1750e-3;
				currentOffPmos[80] = 9.9278e-3;
				currentOffPmos[90] = 1.56146e-2;
				currentOffPmos[100] =2.40633e-2;
				pnSizeRatio = (int)(currentOnNmos[0]/currentOnPmos[0]);
			}
		} 		
		else if (featureSizeInNano >= 1) {
			if (deviceRoadmap == HP) {

				// 1.4 update
				cout<<"HP for 14 nm and beyond is not supported"<<endl;
				exit(-1);

			} else {

				// 1.4 update: IRDS 2022
				vdd = 0.6;
				vth = vth_list[6];
				PitchFin= 24e-9;
				phyGateLength = 1.2e-8;

				// 1.4 update: IRDS 2022 - GAA specfic parameters
				max_fin_per_GAA=1;
				max_sheet_num=4;
				thickness_sheet=6*1e-9;
				width_sheet=10*1e-9;	
					
				widthFin= width_sheet; // for drain height calculation 
				effective_width=(thickness_sheet+width_sheet)*2;
				
				capIdealGate = caplist[6] * 1E-18 /  (effective_width*max_sheet_num) ;
				cap_draintotal = cap_draintotallist[6]/ (effective_width);
				capFringe = 0;

				effectiveResistanceMultiplier = eff_res_mul[6];	/* from CACTI */
				current_gmNmos= gm[6];
				current_gmPmos= gm[6];	
				gm_oncurrent = gm[6];  // gm at on current	

				/* NeuroSim 1.4 Only Conducts TCAD Tuning for Room Temp (300K) On Current. Restrict to Array 14nm -> 1nm in V.1.0 */
				currentOnNmos[0]  = currentlist[6];
				currentOnNmos[10] = currentlist[6];
				currentOnNmos[20] = currentlist[6];
				currentOnNmos[30] = currentlist[6];
				currentOnNmos[40] = currentlist[6];
				currentOnNmos[50] = currentlist[6];
				currentOnNmos[60] = currentlist[6];
				currentOnNmos[70] = currentlist[6];
				currentOnNmos[80] = currentlist[6];
				currentOnNmos[90] = currentlist[6];
				currentOnNmos[100]= currentlist[6];
				currentOnPmos[0]  = currentOnNmos[0]; 
				currentOnPmos[10] = currentOnNmos[0];
				currentOnPmos[20] = currentOnNmos[0];
				currentOnPmos[30] = currentOnNmos[0];
				currentOnPmos[40] = currentOnNmos[0];
				currentOnPmos[50] = currentOnNmos[0];
				currentOnPmos[60] = currentOnNmos[0];
				currentOnPmos[70] = currentOnNmos[0];
				currentOnPmos[80] = currentOnNmos[0];
				currentOnPmos[90] = currentOnNmos[0];
				currentOnPmos[100] =currentOnNmos[0];
				currentOffNmos[0]  = currentlist_off[6]; /* Use off current adjustment from NSv1.5*/
				currentOffNmos[10] = 1.85E-04;
				currentOffNmos[20] = 3.32E-04;
				currentOffNmos[30] = 5.74E-04;
				currentOffNmos[40] = 9.62E-04;
				currentOffNmos[50] = 1.5695e-3;
				currentOffNmos[60] = 2.4953e-3;
				currentOffNmos[70] = 3.8744e-3 ;
				currentOffNmos[80] = 5.8858e-3 ;
				currentOffNmos[90] = 8.7624e-3;
				currentOffNmos[100] =1.28025e-2;
				currentOffPmos[0]  = 100.9536e-6;
				currentOffPmos[10] = 201.3937e-6;
				currentOffPmos[20] = 386.2086e-6;
				currentOffPmos[30] = 714.4288e-6;
				currentOffPmos[40] = 1.2788e-3;
				currentOffPmos[50] = 2.2207e-3;
				currentOffPmos[60] = 3.7509e-3;
				currentOffPmos[70] = 6.1750e-3;
				currentOffPmos[80] = 9.9278e-3;
				currentOffPmos[90] = 1.56146e-2;
				currentOffPmos[100] =2.40633e-2;
				pnSizeRatio = (int)(currentOnNmos[0]/currentOnPmos[0]);
			}
		} else {
			cout<<"Error: CMOS Technology node "<< featureSizeInNano <<"nm is not supported"<<endl;
	        exit(-1);
		}

    // Setup TSV params 
    if (_featureSizeInNano >= 180) {
        // NOTE: 180nm extrapolated from 22,32,45,65, and 90nm values
        //TSV aggressive, projected from ITRS
        tsv_pitch[0][0] = 5.6; // in um
        tsv_diameter[0][0] = 2.8;
        tsv_length[0][0] = 10.0;
        tsv_dielec_thickness[0][0] = 0.1;
        tsv_contact_resistance[0][0] = 0.1; // Ohm
        tsv_depletion_width[0][0] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][0] = 3.3012;

        //TSV conservative, projected from ITRS
        tsv_pitch[1][0] = 8.88; // in um
        tsv_diameter[1][0] = 4.46;
        tsv_length[1][0] = 40.0;
        tsv_dielec_thickness[1][0] = 0.1;
        tsv_contact_resistance[1][0] = 0.1;
        tsv_depletion_width[1][0] = 0.6;
        tsv_liner_dielectric_constant[1][0] = 3.4652;

        //TSV aggressive, industry reported
        tsv_pitch[0][1] = 58.6; // in um
        tsv_diameter[0][1] = 7.12;
        tsv_length[0][1] = 77.0;
        tsv_dielec_thickness[0][1] = 0.26;
        tsv_contact_resistance[0][1] = 0.26; // Ohm
        tsv_depletion_width[0][1] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][1] = 3.3012;

        //TSV conservative, industry reported
        tsv_pitch[1][1] = 104; // in um
        tsv_diameter[1][1] = 11.54;
        tsv_length[1][1] = 85.0;
        tsv_dielec_thickness[1][1] = 0.68;
        tsv_contact_resistance[1][1] = 0.2;
        tsv_depletion_width[1][1] = 0.6;
        tsv_liner_dielectric_constant[1][1] = 3.4652;
    } else if (_featureSizeInNano >= 120) {
        // NOTE: 130nm extrapolated from 22,32,45,65, and 90nm values
        //TSV aggressive, projected from ITRS
        tsv_pitch[0][0] = 4.78; // in um
        tsv_diameter[0][0] = 2.39;
        tsv_length[0][0] = 9.0;
        tsv_dielec_thickness[0][0] = 0.1;
        tsv_contact_resistance[0][0] = 0.1; // Ohm
        tsv_depletion_width[0][0] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][0] = 2.9783;

        //TSV conservative, projected from ITRS
        tsv_pitch[1][0] = 7.7; // in um
        tsv_diameter[1][0] = 3.87;
        tsv_length[1][0] = 35.0;
        tsv_dielec_thickness[1][0] = 0.1;
        tsv_contact_resistance[1][0] = 0.1;
        tsv_depletion_width[1][0] = 0.6;
        tsv_liner_dielectric_constant[1][0] = 3.2264;

        //TSV aggressive, industry reported
        tsv_pitch[0][1] = 49.9; // in um
        tsv_diameter[0][1] = 6.41;
        tsv_length[0][1] = 68.0;
        tsv_dielec_thickness[0][1] = 0.24;
        tsv_contact_resistance[0][1] = 0.24; // Ohm
        tsv_depletion_width[0][1] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][1] = 2.9783;

        //TSV conservative, industry reported
        tsv_pitch[1][1] = 91; // in um
        tsv_diameter[1][1] = 11.41;
        tsv_length[1][1] = 77.5;
        tsv_dielec_thickness[1][1] = 0.62;
        tsv_contact_resistance[1][1] = 0.2;
        tsv_depletion_width[1][1] = 0.6;
        tsv_liner_dielectric_constant[1][1] = 3.2264;
    } else if (_featureSizeInNano >= 90) {
        //TSV aggressive, projected from ITRS
        tsv_pitch[0][0] = 4.0; // in um
        tsv_diameter[0][0] = 2.0;
        tsv_length[0][0] = 8.0;
        tsv_dielec_thickness[0][0] = 0.1;
        tsv_contact_resistance[0][0] = 0.1; // Ohm
        tsv_depletion_width[0][0] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][0] = 2.709;

        //TSV conservative, projected from ITRS
        tsv_pitch[1][0] = 6.9; // in um
        tsv_diameter[1][0] = 3.5;
        tsv_length[1][0] = 30.0;
        tsv_dielec_thickness[1][0] = 0.1;
        tsv_contact_resistance[1][0] = 0.1;
        tsv_depletion_width[1][0] = 0.6;
        tsv_liner_dielectric_constant[1][0] = 3.038;

        //TSV aggressive, industry reported
        tsv_pitch[0][1] = 45; // in um
        tsv_diameter[0][1] = 6.9;
        tsv_length[0][1] = 60.0;
        tsv_dielec_thickness[0][1] = 0.2;
        tsv_contact_resistance[0][1] = 0.2; // Ohm
        tsv_depletion_width[0][1] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][1] = 2.709;

        //TSV conservative, industry reported
        tsv_pitch[1][1] = 90; // in um
        tsv_diameter[1][1] = 11.3;
        tsv_length[1][1] = 75.0;
        tsv_dielec_thickness[1][1] = 0.5;
        tsv_contact_resistance[1][1] = 0.2;
        tsv_depletion_width[1][1] = 0.6;
        tsv_liner_dielectric_constant[1][1] = 3.038;

    } else if (_featureSizeInNano >= 65) {
        //TSV aggressive, projected from ITRS
        tsv_pitch[0][0] = 3.2; // in um
        tsv_diameter[0][0] = 1.6;
        tsv_length[0][0] = 7.0;
        tsv_dielec_thickness[0][0] = 0.1;
        tsv_contact_resistance[0][0] = 0.1; // Ohm
        tsv_depletion_width[0][0] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][0] = 2.303;

        //TSV conservative, projected from ITRS
        tsv_pitch[1][0] = 5; // in um
        tsv_diameter[1][0] = 2.5;
        tsv_length[1][0] = 25.0;
        tsv_dielec_thickness[1][0] = 0.1;
        tsv_contact_resistance[1][0] = 0.1;
        tsv_depletion_width[1][0] = 0.6;
        tsv_liner_dielectric_constant[1][0] = 2.734;

        //TSV aggressive, industry reported
        tsv_pitch[0][1] = 30; // in um
        tsv_diameter[0][1] = 4.6;
        tsv_length[0][1] = 50.0;
        tsv_dielec_thickness[0][1] = 0.2;
        tsv_contact_resistance[0][1] = 0.2; // Ohm
        tsv_depletion_width[0][1] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][1] = 2.303;

        //TSV conservative, industry reported
        tsv_pitch[1][1] = 60; // in um
        tsv_diameter[1][1] = 7.5;
        tsv_length[1][1] = 62.5;
        tsv_dielec_thickness[1][1] = 0.5;
        tsv_contact_resistance[1][1] = 0.2;
        tsv_depletion_width[1][1] = 0.6;
        tsv_liner_dielectric_constant[1][1] = 2.734;

    } else if (_featureSizeInNano >= 45) {
        //TSV aggressive, projected from ITRS
        tsv_pitch[0][0] = 2.2; // in um
        tsv_diameter[0][0] = 1.1;
        tsv_length[0][0] = 6.0;
        tsv_dielec_thickness[0][0] = 0.1;
        tsv_contact_resistance[0][0] = 0.1; // Ohm
        tsv_depletion_width[0][0] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][0] = 1.958;

        //TSV conservative, projected from ITRS
        tsv_pitch[1][0] = 3.4; // in um
        tsv_diameter[1][0] = 1.7;
        tsv_length[1][0] = 20.0;
        tsv_dielec_thickness[1][0] = 0.1;
        tsv_contact_resistance[1][0] = 0.1;
        tsv_depletion_width[1][0] = 0.6;
        tsv_liner_dielectric_constant[1][0] = 2.460;

        //TSV aggressive, industry reported
        tsv_pitch[0][1] = 20; // in um
        tsv_diameter[0][1] = 3.1;
        tsv_length[0][1] = 40.0;
        tsv_dielec_thickness[0][1] = 0.2;
        tsv_contact_resistance[0][1] = 0.2; // Ohm
        tsv_depletion_width[0][1] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][1] = 1.958;

        //TSV conservative, industry reported
        tsv_pitch[1][1] = 40; // in um
        tsv_diameter[1][1] = 5;
        tsv_length[1][1] = 50.0;
        tsv_dielec_thickness[1][1] = 0.5;
        tsv_contact_resistance[1][1] = 0.2;
        tsv_depletion_width[1][1] = 0.6;
        tsv_liner_dielectric_constant[1][1] = 2.460;

    } else if (_featureSizeInNano >= 32) {
        //TSV aggressive, projected from ITRS
        tsv_pitch[0][0] = 1.4; // in um
        tsv_diameter[0][0] = 0.7;
        tsv_length[0][0] = 5.0;
        tsv_dielec_thickness[0][0] = 0.1;
        tsv_contact_resistance[0][0] = 0.1; // Ohm
        tsv_depletion_width[0][0] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][0] = 1.664;

        //TSV conservative, projected from ITRS
        tsv_pitch[1][0] = 4; //2.2; // in um
        tsv_diameter[1][0] = 2; //1.1;
        tsv_length[1][0] = 15.0;
        tsv_dielec_thickness[1][0] = 0.1;
        tsv_contact_resistance[1][0] = 0.1;
        tsv_depletion_width[1][0] = 0.6;
        tsv_liner_dielectric_constant[1][0] = 2.214;

        //TSV aggressive, industry reported
        tsv_pitch[0][1] = 15; // in um
        tsv_diameter[0][1] = 2.3;
        tsv_length[0][1] = 30.0;
        tsv_dielec_thickness[0][1] = 0.2;
        tsv_contact_resistance[0][1] = 0.2; // Ohm
        tsv_depletion_width[0][1] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][1] = 1.664;

        //TSV conservative, industry reported
        tsv_pitch[1][1] = 30; // in um
        tsv_diameter[1][1] = 3.8;
        tsv_length[1][1] = 37.5;
        tsv_dielec_thickness[1][1] = 0.5;
        tsv_contact_resistance[1][1] = 0.2;
        tsv_depletion_width[1][1] = 0.6;
        tsv_liner_dielectric_constant[1][1] = 2.214;

    } else if (_featureSizeInNano >= 22) {
        //TSV aggressive, projected from ITRS
        tsv_pitch[0][0] = 0.8; // in um
        tsv_diameter[0][0] = 0.4;
        tsv_length[0][0] = 4.0;
        tsv_dielec_thickness[0][0] = 0.1;
        tsv_contact_resistance[0][0] = 0.1; // Ohm
        tsv_depletion_width[0][0] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][0] = 1.414;

        //TSV conservative, projected from ITRS
        tsv_pitch[1][0] = 1.5; // in um
        tsv_diameter[1][0] = 0.8;
        tsv_length[1][0] = 10.0;
        tsv_dielec_thickness[1][0] = 0.1;
        tsv_contact_resistance[1][0] = 0.1;
        tsv_depletion_width[1][0] = 0.6;
        tsv_liner_dielectric_constant[1][0] = 2.104;

        //TSV aggressive, industry reported
        tsv_pitch[0][1] = 9; // in um
        tsv_diameter[0][1] = 4.5;
        tsv_length[0][1] = 25.0;
        tsv_dielec_thickness[0][1] = 0.1;
        tsv_contact_resistance[0][1] = 0.1;
        tsv_depletion_width[0][1] = 0.6;
        tsv_liner_dielectric_constant[0][1] = 1.414;

        //TSV conservative, industry reported -- Swapped from CACTI3DD
        tsv_pitch[1][1] = 40; // in um
        tsv_diameter[1][1] = 7.5;
        tsv_length[1][1] = 50.0;
        tsv_dielec_thickness[1][1] = 0.2;
        tsv_contact_resistance[1][1] = 0.2; // Ohm
        tsv_depletion_width[1][1] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[1][1] = 2.104;
    } /*TODO: ADD NEW PROJECTIONS, EVERYTHING SEEN IS TEMPORARY*/ else if (_featureSizeInNano >= 1) {
        //TSV aggressive, projected from IRDS
        tsv_pitch[0][0] = 4; // in um
        tsv_diameter[0][0] = 1;
        tsv_length[0][0] = 10.0;
        tsv_dielec_thickness[0][0] = 0.1;
        tsv_contact_resistance[0][0] = 0.7;
        tsv_depletion_width[0][0] = 0.6;
        tsv_liner_dielectric_constant[0][0] = 2.4;

        //TSV conservative, projected from IRDS
        tsv_pitch[1][0] = 10; // in um
        tsv_diameter[1][0] = 4.5;
        tsv_length[1][0] = 50.0;
        tsv_dielec_thickness[1][0] = 0.2;
        tsv_contact_resistance[1][0] = 0.9; // Ohm
        tsv_depletion_width[1][0] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[1][0] = 2.55;

        //TSV aggressive, industry reported
        tsv_pitch[0][1] = 4; // in um
        tsv_diameter[0][1] = 1;
        tsv_length[0][1] = 10.0;
        tsv_dielec_thickness[0][1] = 0.1;
        tsv_contact_resistance[0][1] = 0.7;
        tsv_depletion_width[0][1] = 0.6;
        tsv_liner_dielectric_constant[0][1] = 2.4;

        //TSV conservative, industry reported -- Swapped from CACTI3DD
        tsv_pitch[1][1] = 10; // in um
        tsv_diameter[1][1] = 4.5;
        tsv_length[1][1] = 50.0;
        tsv_dielec_thickness[1][1] = 0.2;
        tsv_contact_resistance[1][1] = 0.9; // Ohm
        tsv_depletion_width[1][1] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[1][1] = 2.55;

		//MIV - From Prof. SK Lim's Group - Use same projection
        tsv_pitch[0][2] = 0.1; // in um
        tsv_diameter[0][2] = 0.05;
        tsv_length[0][2] = 0.1;
        tsv_dielec_thickness[0][2] = 0.01;
        tsv_contact_resistance[0][2] = 1; // Ohm
        tsv_depletion_width[0][2] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[0][2] = 2.55;

		//MIV - From Prof. SK Lim's Group - Use same projection
        tsv_pitch[1][2] = 0.1; // in um
        tsv_diameter[1][2] = 0.05;
        tsv_length[1][2] = 0.1;
        tsv_dielec_thickness[1][2] = 0.01;
        tsv_contact_resistance[1][2] = 1; // Ohm
        tsv_depletion_width[1][2] = 0.6;  // um, Calculated from cylindrical Possion equation
        tsv_liner_dielectric_constant[1][2] = 2.55;
    }
    // Note: CACTI3DD also has 16nm
    
    // Initialize to something -- will be changed in main loop later.
    SetLayerCount(inputParameter, 2);

    /* For non-DRAM types vpp is equal to vdd. */
    if (_deviceRoadmap != EDRAM) {
        vpp = vdd;
    }

	if (_featureSizeInNano >= 22) {
		capOverlap = capIdealGate * 0.2;
	} else {
		capOverlap = 0;	// capOverlap and capFringe are included in capIdealGate in FinFET technology, so we let these two parameters 0
	}
	//capSidewall = 2.5e-10;	/* Unit: F/m, this value is from CACTI, PTM model shows the value is 5e-10 */
	double cjd = 1e-3;             /* Bottom junction capacitance, Unit: F/m^2*/
	double cjswd = 2.5e-10;           /* Isolation-edge sidewall junction capacitance, Unit: F/m */
	double cjswgd = 0.5e-10;          /* Gate-edge sidewall junction capacitance, Unit: F/m */
	double mjd = 0.5;             /* Bottom junction capacitance grating coefficient */
	double mjswd = 0.33;           /* Isolation-edge sidewall junction capacitance grading coefficient */
	double mjswgd = 0.33;          /* Gate-edge sidewall junction capacitance grading coefficient */
	buildInPotential = 0.9;			/* This value is from BSIM4 */

	if ( _featureSizeInNano == 14 ) capJunction = 0.0120;
	else if ( _featureSizeInNano == 10 ) capJunction = 0.0134;
	else if ( _featureSizeInNano== 7 ) capJunction = 0.0137;
	else if ( _featureSizeInNano == 5 ) capJunction = 0.0119;
	else if ( _featureSizeInNano == 3 ) capJunction = 0.0128;
	else if ( _featureSizeInNano == 2 ) capJunction = 0.0091;
	else if ( _featureSizeInNano == 1 ) capJunction = 0.0102;
	else capJunction = cjd / pow(1 + vdd / buildInPotential, mjd);

	capSidewall = cjswd / pow(1 + vdd / buildInPotential, mjswd);
	capDrainToChannel = cjswgd / pow(1 + vdd / buildInPotential, mjswgd);

	vdsatNmos = phyGateLength * 1e5 /* Silicon saturatio velocity, Unit: m/s */ / effectiveElectronMobility;
	vdsatPmos = phyGateLength * 1e5 /* Silicon saturatio velocity, Unit: m/s */ / effectiveHoleMobility;

	/* Properties not used so far */
	capPolywire = 0.0;	/* TO-DO: we need to find the values */

	/* Interpolate */
	for (int i = 1; i < 100; i++) {
		if (i % 10) {
			double a = currentOnNmos[i / 10 * 10];
			double b = currentOnNmos[i / 10 * 10 + 10];
			currentOnNmos[i] = a + (b-a) * (i % 10) / 10;
		}
	}

	for (int i = 1; i < 100; i++) {
		if (i % 10) {
			double a = currentOnPmos[i / 10 * 10];
			double b = currentOnPmos[i / 10 * 10 + 10];
			currentOnPmos[i] = a + (b-a) * (i % 10) / 10;
		}
	}

	for (int i = 1; i < 100; i++) {
		if (i % 10) {
			double a = currentOffNmos[i / 10 * 10];
			double b = currentOffNmos[i / 10 * 10 + 10];
			currentOffNmos[i] = a + (b-a) * (i % 10) / 10;
		}
	}

	for (int i = 1; i < 100; i++) {
		if (i % 10) {
			double a = currentOffPmos[i / 10 * 10];
			double b = currentOffPmos[i / 10 * 10 + 10];
			currentOffPmos[i] = a + (b-a) * (i % 10) / 10;
		}
	}

	neurosim_wiring = true;
	initialized = true;
}

void Technology::PrintProperty() {
	cout << "Fabrication Process Technology Node:" << endl;
	cout << "TO-DO" << endl;
}

void Technology::InterpolateWith(Technology rhs, double _alpha) {
	if (featureSizeInNano != rhs.featureSizeInNano) {
		vdd = (1 - _alpha) * vdd + _alpha * rhs.vdd;
		vth = (1 - _alpha) * vth + _alpha * rhs.vth;
		phyGateLength = (1 - _alpha) * phyGateLength + _alpha * rhs.phyGateLength;
		capIdealGate = (1 - _alpha) * capIdealGate + _alpha * rhs.capIdealGate;
		capFringe = (1 - _alpha) * capFringe + _alpha * rhs.capFringe;
		capJunction = (1 - _alpha) * capJunction + _alpha * rhs.capJunction;
		capOx = (1 - _alpha) * capOx + _alpha * rhs.capOx;
		effectiveElectronMobility = (1 - _alpha) * effectiveElectronMobility + _alpha * rhs.effectiveElectronMobility;
		effectiveHoleMobility = (1 - _alpha) * effectiveHoleMobility + _alpha * rhs.effectiveHoleMobility;
		pnSizeRatio = (1 - _alpha) * pnSizeRatio + _alpha * rhs.pnSizeRatio;
		effectiveResistanceMultiplier = (1 - _alpha) * effectiveResistanceMultiplier + _alpha * rhs.effectiveResistanceMultiplier;
		for (int i = 0; i <= 100; i++){
			currentOnNmos[i] = (1 - _alpha) * currentOnNmos[i] + _alpha * rhs.currentOnNmos[i];
			currentOnPmos[i] = (1 - _alpha) * currentOnPmos[i] + _alpha * rhs.currentOnPmos[i];
			currentOffNmos[i] = pow(currentOffNmos[i], 1 - _alpha) * pow(rhs.currentOffNmos[i], _alpha);
			currentOffPmos[i] = pow(currentOffPmos[i], 1 - _alpha) * pow(rhs.currentOffPmos[i], _alpha);
		}
		//capSidewall = 2.5e-10;	/* Unit: F/m, this value is from CACTI, PTM model shows the value is 5e-10 */
		double cjd = 1e-3;             /* Bottom junction capacitance, Unit: F/m^2*/
		double cjswd = 2.5e-10;           /* Isolation-edge sidewall junction capacitance, Unit: F/m */
		double cjswgd = 0.5e-10;          /* Gate-edge sidewall junction capacitance, Unit: F/m */
		double mjd = 0.5;             /* Bottom junction capacitance grating coefficient */
		double mjswd = 0.33;           /* Isolation-edge sidewall junction capacitance grading coefficient */
		double mjswgd = 0.33;          /* Gate-edge sidewall junction capacitance grading coefficient */
		buildInPotential = 0.9;			/* This value is from BSIM4 */
		capJunction = cjd / pow(1 + vdd / buildInPotential, mjd);
		capSidewall = cjswd / pow(1 + vdd / buildInPotential, mjswd);
		capDrainToChannel = cjswgd / pow(1 + vdd / buildInPotential, mjswgd);

		vdsatNmos = phyGateLength * 1e5 /* Silicon saturatio velocity, Unit: m/s */ / effectiveElectronMobility;
		vdsatPmos = phyGateLength * 1e5 /* Silicon saturatio velocity, Unit: m/s */ / effectiveHoleMobility;

		// new technology parameters
		max_sheet_num = max_sheet_num;
		thickness_sheet = thickness_sheet;
		width_sheet = width_sheet;
		effective_width = effective_width;
		max_fin_num = max_fin_num;
		max_fin_per_GAA = rhs.max_fin_per_GAA;
		gm_oncurrent = (1 - _alpha) * gm_oncurrent + _alpha * rhs.gm_oncurrent;  // gm at on current
		cap_draintotal = (1 - _alpha) * cap_draintotal + _alpha * rhs.cap_draintotal;
		current_gmNmos = (1 - _alpha) * current_gmNmos + _alpha * rhs.current_gmNmos;		/* NMOS current at 0.7*vdd for gm calculation, Unit: A/m/V*/ 
    	current_gmPmos = (1 - _alpha) * current_gmPmos + _alpha * rhs.current_gmPmos;		/* PMOS current at 0.7*vdd for gm calculation, Unit: A/m/V*/
		heightFin = heightFin;	/* Fin height, Unit: m */
		widthFin = widthFin;	/* Fin width, Unit: m */
		PitchFin = PitchFin;	/* Fin pitch, Unit: m */
	}
}

double Technology::tsv_calc_depwidth(double interface_potential, double t_radius)
{
	// Define Constants First
	double epsilon_0 = 8.854e-12;
    double epsilon_si = 11.7 * epsilon_0;
    double k_boltzman = 1.381e-23;
    double Na_dopant = 0.137e22;
    double carrier_temp = 9.15e19 * (pow((350/300),2.0)) * std::exp((-6880/350));
    double elementary_q = 1.602e-19;

	// Time Constant Calculation
    double tau_a = -1 * elementary_q * Na_dopant / epsilon_si;
    double tau_b = std::exp((-1 * elementary_q * interface_potential / (k_boltzman * 350)));
    double tau_c = pow((carrier_temp/Na_dopant),2.0);
    double tau_s = tau_a * (tau_b - 1 + tau_c);

    // Nu Calculation
    double nu_a = k_boltzman * 350 * Na_dopant / epsilon_si;
    double nu_b = elementary_q * interface_potential / (k_boltzman * 350);
    double nu_s = nu_a * ((tau_b + nu_b - 1) - (tau_c * nu_b));

    // Gamma Calculation
    double gamma_a = interface_potential + (pow(t_radius,2.0) * tau_s);
    double gamma_b = sqrt(pow(interface_potential,2.0) + (2 * pow(t_radius,2.0) * nu_s));
    double gamma_s = 1 + (gamma_a / gamma_b);

    // Wdep Calculation
    return t_radius * (std::exp((1/gamma_s)) - 1);
}

double Technology::tsv_resistance(double resistivity, double tsv_len, double tsv_diam, double tsv_contact_resistance)
{
	double resistance;
	resistance = resistivity * tsv_len / (3.1416 * (tsv_diam/2) * (tsv_diam/2)) + tsv_contact_resistance;
	return(resistance);
}

double Technology::tsv_capacitance(double tsv_len, double tsv_diam, double tsv_pitch, double dielec_thickness, double liner_dielectric_constant, double depletion_width)
{
	double self_cap, liner_cap, depletion_cap, lateral_coupling_cap, diagonal_coupling_cap, total_cap;
	double diagonal_coupling_constant, lateral_coupling_constant;
	const double e_si = PERMITTIVITY_FREE_SPACE * 11.9, PI = 3.1416;
	lateral_coupling_constant = 4.1;
	diagonal_coupling_constant = 5.3;
	//depletion_width = 0.6; // um -> Destiny Assumption. Use KAIST calculation
	depletion_width = tsv_calc_depwidth(vdd, tsv_diam/2 * 1e-6) * 1e6; // Setup need in microns
	liner_cap = 2 * PI * PERMITTIVITY_FREE_SPACE * liner_dielectric_constant * tsv_len / log(1 + dielec_thickness / (tsv_diam/2));
	depletion_cap = 2 * PI * e_si *tsv_len / log(1 + depletion_width / (dielec_thickness + tsv_diam/2));
	//self_cap = ( 1 / (1/liner_cap + 1/depletion_cap) + liner_cap ) / 2;
	self_cap = 1 / (1/liner_cap + 1/depletion_cap);


#ifdef NVSIM3DDEBUG
    cout<<"TSV ox cap: "<<liner_cap*1e15<<" fF"<<endl;
    cout<<"TSV self cap: "<<self_cap*1e15<<" fF"<<endl;
#endif

	lateral_coupling_cap = 0.4 *  (0.225 * log(0.97 * tsv_len / tsv_diam) + 0.53) * e_si / (tsv_pitch - tsv_diam) * PI * tsv_diam * tsv_len;
	diagonal_coupling_cap = 0.4 *  (0.225 * log(0.97 * tsv_len / tsv_diam) + 0.53) * e_si / (1.414 * tsv_pitch - tsv_diam) * PI * tsv_diam * tsv_len;
	total_cap = self_cap + lateral_coupling_constant * lateral_coupling_cap + diagonal_coupling_constant * diagonal_coupling_cap;
	return(total_cap);
}

double Technology::tsv_area(double tsv_pitch)
{
	return(pow(tsv_pitch,2));
}

// Use to determine TSV type. All aggressive type interconnects use aggressive TSVs
// Returns 0 - Aggressive, 1 - Conservative
TSV_type Technology::WireTypeToTSVType(int wiretype)
{
    TSV_type rv = Fine;

    switch (wiretype) {
        case local_aggressive:
        case semi_aggressive:
        case global_aggressive:
            rv = Fine;
            break;
        case local_conservative:
        case semi_conservative:
        case global_conservative:
        case dram_wordline:
            rv = Coarse;
            break;
        default:
            rv = Fine;
            break;
    }

    return rv;
}

void Technology::SetLayerCount(InputParameter *inputParameter, int layers)
{
    double length_value;

    if (layers == layerCount)
        return;

    //TSV aggressive, projected from ITRS
    length_value = tsv_length[0][0] * layers;

    //cout << "tsv_length " << length_value << " tsv_diameter " << tsv_diameter[0][0] << " tsv_contact_resistance " << tsv_contact_resistance[0][0] << std::endl;

    tsv_parasitic_res[0][0] = tsv_resistance(BULK_CU_RESISTIVITY, length_value, tsv_diameter[0][0], tsv_contact_resistance[0][0]);
    tsv_parasitic_cap[0][0] = tsv_capacitance(length_value, tsv_diameter[0][0], tsv_pitch[0][0], tsv_dielec_thickness[0][0], tsv_liner_dielectric_constant[0][0], tsv_depletion_width[0][0]);
    tsv_occupation_area[0][0] = tsv_area(tsv_pitch[0][0]);

    //TSV conservative, projected from ITRS
    length_value = tsv_length[1][0] * layers;
    tsv_parasitic_res[1][0] = tsv_resistance(BULK_CU_RESISTIVITY, length_value, tsv_diameter[1][0], tsv_contact_resistance[1][0]);
    tsv_parasitic_cap[1][0] = tsv_capacitance(length_value, tsv_diameter[1][0], tsv_pitch[1][0], tsv_dielec_thickness[1][0], tsv_liner_dielectric_constant[1][0], tsv_depletion_width[1][0]);
    tsv_occupation_area[1][0] = tsv_area(tsv_pitch[1][0]);

    //TSV aggressive, industry reported
    length_value = tsv_length[0][1] * layers;
    tsv_parasitic_res[0][1] = tsv_resistance(BULK_CU_RESISTIVITY, length_value, tsv_diameter[0][1], tsv_contact_resistance[0][1]);
    tsv_parasitic_cap[0][1] = tsv_capacitance(length_value, tsv_diameter[0][1], tsv_pitch[0][1], tsv_dielec_thickness[0][1], tsv_liner_dielectric_constant[0][1], tsv_depletion_width[0][1]);
    tsv_occupation_area[0][1] = tsv_area(tsv_pitch[0][1]);

    //TSV conservative, industry reported
    length_value = tsv_length[1][1] * layers;
    tsv_parasitic_res[1][1] = tsv_resistance(BULK_CU_RESISTIVITY, length_value, tsv_diameter[1][1], tsv_contact_resistance[1][1]);
    tsv_parasitic_cap[1][1] = tsv_capacitance(length_value, tsv_diameter[1][1], tsv_pitch[1][1], tsv_dielec_thickness[1][1], tsv_liner_dielectric_constant[1][1], tsv_depletion_width[1][1]);
    tsv_occupation_area[1][1] = tsv_area(tsv_pitch[1][1]);

	//MIV aggressive/conservative
	length_value = tsv_length[0][2] * layers;
    tsv_parasitic_res[0][2] = 10;
    tsv_parasitic_cap[0][2] = 0.2e-15;
    tsv_occupation_area[0][2] = tsv_area(tsv_pitch[0][2]);

	//MIV aggressive/conservative
	length_value = tsv_length[1][2] * layers;
    tsv_parasitic_res[1][2] = 32;
    tsv_parasitic_cap[1][2] = 0.64e-15;
    tsv_occupation_area[1][2] = tsv_area(tsv_pitch[1][2]);

    //Finalize TSV parameters in tech pointer
    int local_ic_proj_type = WireTypeToTSVType(inputParameter->maxLocalWireType);
    int global_ic_proj_type = WireTypeToTSVType(inputParameter->maxGlobalWireType);
    int tsv_is_mat_type = inputParameter->localTsvProjection;
    int tsv_os_bank_type = inputParameter->globalTsvProjection;

    //for (int i=0; i < 2; i++) {
    //    for (int j=0; j < 2; j++) {
    //        cout << "res[" << i << "][" << j << "] = " << tsv_parasitic_res[i][j] << endl;
    //        cout << "cap[" << i << "][" << j << "] = " << tsv_parasitic_cap[i][j] * 1e14 << "e-14" << endl;
    //        cout << "area[" << i << "][" << j << "] = " << tsv_occupation_area[i][j] << endl;
    //    }
    //}

    resTSV[Fine] = tsv_parasitic_res[local_ic_proj_type][tsv_is_mat_type];
    capTSV[Fine] = tsv_parasitic_cap[local_ic_proj_type][tsv_is_mat_type];
    areaTSV[Fine] = tsv_occupation_area[local_ic_proj_type][tsv_is_mat_type];

    resTSV[Coarse] = tsv_parasitic_res[global_ic_proj_type][tsv_os_bank_type];
    capTSV[Coarse] = tsv_parasitic_cap[global_ic_proj_type][tsv_os_bank_type];
    areaTSV[Coarse] = tsv_occupation_area[global_ic_proj_type][tsv_os_bank_type];

	resTSV[Monolithic] = tsv_parasitic_res[0][2];
    capTSV[Monolithic] = tsv_parasitic_cap[0][2];
    areaTSV[Monolithic] = tsv_occupation_area[0][2];

    layerCount = layers;
}

