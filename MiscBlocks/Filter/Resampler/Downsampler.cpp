/*
 *  Created on: Oct 24, 2018
 *      Author: bertold
 */
#include "Downsampler.h"

#include <iostream>

namespace Filter {
namespace Resampler {

/* This will be in std::numeric in C++17 :) */
int gcd(unsigned int v1, unsigned int v2) {
	while (v2) {
		int tmp = v2;
		v2 = v1 % v2;
		v1 = tmp;
	}
	return v1;
}

}
} /* namespace Filter */
