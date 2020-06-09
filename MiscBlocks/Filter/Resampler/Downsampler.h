/*
 *  Created on: Oct 24, 2018
 *      Author: bertold
 */

#include <cmath>
#include <cstdint>

#include "../CIC/CIC.h"
#include "../FIR/FIRDesign.h"
#include "../FIR/RationalFIRFilter.h"
#include "../FIR/FractionalFIRFilter.h"

#include "Resampler.h"

#include <iostream>
#include <functional>
#include <stdexcept>

#ifndef FILTER_RESAMPLER_DOWNSAMPLER_H_
#define FILTER_RESAMPLER_DOWNSAMPLER_H_

namespace Filter {
namespace Resampler {

int gcd(unsigned int v1, unsigned int v2);


template <typename inT, typename outT, typename cicInternalType = int32_t> class Downsampler {
public:
	Downsampler(ResamplerConfig cfg, unsigned int maxBlockLength = 256*1024):
			cfg_(cfg){

		/* Create working buffer */
		samplesCIC_ = new float[maxBlockLength];

		/*
		* The CIC filter makes half band filters, so we use it
		* to get the rate to 2-4x of the output rate
		*/
		unsigned int cicD = 1;
		while((cfg_.inputRate / cicD) >= cfg_.cicOversampleFactor*cfg_.outputRate){
			cicR_++;
			cicD *= 2;
		}

		if(cicR_ == 1){
			/* Avoid useless work */
			cfg_.cicOrder = 0;
		}

		float firInputRate = cfg.inputRate / cicD;


		/* Create CIC core */
		cic_ = CIC::CIC<inT, float, cicInternalType>(cfg_.cicOrder, cicD);
		std::function<float(float)> cicGainFunc = [this](float f){ return 1.0f/cic_.getGain(f); };
		actualOutputRate_ = cfg_.inputRate / (float)cicD;

		if(cicR_ == 1){
			/* If we don't supply the gain function the integral is solved analytically */
			cicGainFunc = nullptr;
		}

		if(cfg_.rational){
			unsigned int interpolation = cfg_.outputRate;
			unsigned int decimation = firInputRate;

			/* Reduce fraction */
			unsigned int d = gcd(decimation, interpolation);
			decimation /= d;
			interpolation /= d;

			if(interpolation > cfg_.maxInterpolation){
				throw std::runtime_error("Interpolation factor exceeds maximum!");
			}

			actualOutputRate_*= interpolation;
			actualOutputRate_/= decimation;

			/* Create FIR core */
			auto taps = FIR::Design::buildFIRFilter(actualOutputRate_ * cfg_.cutoffFraction,
													 actualOutputRate_ * cfg_.transitionWidthFraction,
													 firInputRate * interpolation,
													 cfg_.desiredRipple,
													 cicGainFunc);

			for(auto& tap: taps){
				tap *= interpolation;
			}

			rationalFir_ = FIR::RationalFIR<float, float, outT>(taps, interpolation, decimation);
		}else{
			unsigned int interpolation = cfg_.maxInterpolation;
			float decimation = firInputRate * interpolation / cfg_.outputRate;

			actualOutputRate_*= interpolation;
			actualOutputRate_/= decimation;

			/* Create FIR core */
			auto taps = FIR::Design::buildFIRFilter(actualOutputRate_ * cfg_.cutoffFraction,
													 actualOutputRate_ * cfg_.transitionWidthFraction,
													 firInputRate * interpolation,
													 cfg_.desiredRipple,
													 cicGainFunc);

			for(auto& tap: taps){
				tap *= interpolation;
			}

			fractionalFir_ = FIR::FrationalFIR<float, float, outT>(taps, interpolation, decimation);
		}
	}

	unsigned int filter(inT* samplesIn, unsigned int numSamples, outT* samplesOut){
		unsigned int samplesAfterCIC;

		if(cicR_ != 1){
			samplesAfterCIC = cic_.filter(samplesIn, numSamples, samplesCIC_);

			auto growth = cic_.getGrowth();

			for(unsigned int i=0; i<samplesAfterCIC; i++){
				samplesCIC_[i] /= growth;
			}
		}else{
			samplesAfterCIC = numSamples;
			for(unsigned int i=0; i<numSamples; i++){
				samplesCIC_[i] = samplesIn[i];
			}
		}

		if(cfg_.rational){
			return rationalFir_.filter(samplesCIC_, samplesAfterCIC, samplesOut);
		}else{
			return fractionalFir_.filter(samplesCIC_, samplesAfterCIC, samplesOut);
		}
	}

	float getActualOutputRate(){
		return actualOutputRate_;
	}

	~Downsampler(){
		delete samplesCIC_;
	}

private:
	CIC::CIC<inT, float, cicInternalType> cic_;

	/* C++ does not support templated virtual functions :( */
	FIR::RationalFIR<float, float, outT> rationalFir_;
	FIR::FrationalFIR<float, float, outT> fractionalFir_;

	ResamplerConfig cfg_;
	float actualOutputRate_;

	float* samplesCIC_;

	unsigned int cicR_ = 1;
};

}
} /* namespace Filter */

#endif /* FILTER_RESAMPLER_DOWNSAMPLER_H_ */
