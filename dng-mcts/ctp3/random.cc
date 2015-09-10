/*
 * random.cpp
 *
 *  Created on: May 4, 2013
 *      Author: baj
 */


#include "random.h"
#include "statistic.h"

double NormalGammaInfo::ALPHA = 0.01;
double NormalGammaInfo::BETA = 100.0;
boost::mt19937 RNG;
