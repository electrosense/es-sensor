/*
 *  Created on: Oct 24, 2018
 *      Author: bertold
 */

#ifndef FILTER_RESAMPLER_RESAMPLER_H_
#define FILTER_RESAMPLER_RESAMPLER_H_

namespace Filter {
namespace Resampler {

struct ResamplerConfig {
	bool rational = true;
	float inputRate;
	float outputRate;
	unsigned int maxInterpolation = 32;
	unsigned int cicOrder = 6;
	unsigned int cicOversampleFactor = 4;

	float desiredRipple = -60;
	float transitionWidthFraction = 0.2;
	float cutoffFraction = 0.3;
};

}
}


#endif /* FILTER_RESAMPLER_RESAMPLER_H_ */
