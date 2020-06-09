/*
 *  Created on: Oct 24, 2018
 *      Author: bertold
 */

#ifndef FIR_FIRDESIGN_H_
#define FIR_FIRDESIGN_H_

#include <tuple>
#include <functional>
#include <vector>

namespace Filter {
namespace FIR {
namespace Design {

std::vector<float> buildKaiser(float rip, float tw, float fs);
std::vector<float> buildFIRBase(float fc, float fs, unsigned int taps, std::function<float(float)> gainFunc = nullptr, float step = 0.0005);

std::vector<float> buildFIRFilter(float fc,
								  float tw = 0.1f,
								  float fs = 1.0f,
								  float rip = -60,
								  std::function<float(float)> gainFunc = nullptr,
								  float step = 0.0005);
}
}
}


#endif /* FIR_FIRDESIGN_H_ */
