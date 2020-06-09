/*
 *  Created on: Oct 24, 2018
 *      Author: bertold
 */

#ifndef FIR_RATIONALFIRFILTER_H_
#define FIR_RATIONALFIRFILTER_H_

#include <vector>

namespace Filter {
namespace FIR {

template <typename tapType, typename inType, typename outType> class RationalFIR {
public:
	RationalFIR():
		taps_({1.0f}),
		rateI_(1),
		rateD_(1){}

	RationalFIR(std::vector<tapType> taps, unsigned int rateI = 1, unsigned int rateD = 1):
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
				/* Run filter */
				samplesOut[numOut] = calcPolyphase();

				outIndex_ = outIndex_ + rateD_;
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
	inline inType calcPolyphase(){
		unsigned int fIndex = delayIndex_;
		inType o = 0;

		for(unsigned int j=0; j<delayLine_.size(); j++){
            o = o + delayLine_[fIndex]*taps_[outIndex_ + rateI_ * j];

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
	unsigned int rateD_;
	std::vector<inType> delayLine_;

	unsigned int delayIndex_ = 0;
	unsigned int outIndex_ = 0;
};

} /* namespace Fir */
} /* namespace Filter */

#endif /* FIR_RATIONALFIRFILTER_H_ */
