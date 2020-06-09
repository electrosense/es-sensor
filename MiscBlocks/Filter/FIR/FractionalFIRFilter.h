/*
 * FractionalFIRFilter.h
 *
 *  Created on: Oct 24, 2018
 *      Author: bertold
 */

#ifndef FILTER_FIR_FRACTIONALFIRFILTER_H_
#define FILTER_FIR_FRACTIONALFIRFILTER_H_

#include <vector>
#include <iostream>

namespace Filter {
namespace FIR {

template <typename tapType, typename inType, typename outType> class FrationalFIR {
public:
	FrationalFIR():
		taps_({1.0f}),
		rateI_(1),
		rateD_(1){}

	FrationalFIR(std::vector<tapType> taps, unsigned int rateI = 1, float rateD = 1.0f):
			taps_(taps),
			rateI_(rateI),
			rateD_(rateD){

		/* Zero pad tabs_ */
		unsigned int extraTaps = (taps_.size() % rateI);
		taps_.resize(taps_.size() + extraTaps);

		delayLine_ = std::vector<inType>(taps_.size() / rateI);
	}

	unsigned int filter(inType* samplesIn, unsigned int numSamples, outType* samplesOut){
		unsigned int numOut = 0;

		for(unsigned int i=0; i<numSamples; i++){
			delayLine_[delayIndex_] = samplesIn[i];

			while(outIndex_ < rateI_){
				/* Interpolate between two polyphase taps */
				float o1 = calcPolyphase(0);
				float o2 = calcPolyphase(1);

				float remainder = outIndex_ - (int)outIndex_;

				float o = remainder * o2 + (1-remainder) * o1;

				outIndex_ = outIndex_ + rateD_;
				samplesOut[numOut] = o;
				numOut++;
			}

			outIndex_ = outIndex_ - rateI_;

			delayIndex_ ++;
			if(delayIndex_ >= delayLine_.size()){
				delayIndex_ = 0;
			}
		}

		return numOut;
	}

private:
	inline inType calcPolyphase(unsigned int offset){
		unsigned int fIndex = delayIndex_;
		unsigned int outIndex = outIndex_;
		inType o = 0;

		outIndex += offset;
		if(outIndex >= rateI_){
			outIndex = 0;
		}

		for(unsigned int j=0; j<delayLine_.size(); j++){
            o = o + delayLine_[fIndex]*taps_[outIndex + rateI_ * j];

            if(!fIndex){
            	fIndex = delayLine_.size() - 1;
            }else{
            	fIndex--;
            }
		}

		return o;
	}

	std::vector<tapType> taps_;
	unsigned int rateI_;
	float rateD_;
	std::vector<inType> delayLine_;

	unsigned int delayIndex_ = 0;
	float outIndex_ = 0;
};

} /* namespace Fir */
} /* namespace Filter */



#endif /* FILTER_FIR_FRACTIONALFIRFILTER_H_ */
