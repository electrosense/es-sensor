/*
 *  Created on: Oct 24, 2018
 *      Author: bertold
 */

#ifndef FILTER_CIC_CIC_H_
#define FILTER_CIC_CIC_H_

#include "IntComb.h"
#include <array>
#include <cmath>

namespace Filter{
namespace CIC{

template <typename inType, typename outType, typename internalType = int32_t, unsigned int maxOrder = 16> class CIC {
public:
	CIC(){}

	CIC(unsigned int order, unsigned int rate):
			order_(order),
			rate_(rate){
		gain_ = 1;
		for(unsigned int i=0; i<order; i++){
			gain_ *= rate;
		}
	}

	internalType getGrowth() const{
		return gain_;
	}

	float getGain(float outputFrequency){
		if(outputFrequency == 0){
			return 1;
		}

		float tmp = sinf(M_PI * outputFrequency);
		tmp /= sinf(M_PI/rate_ * outputFrequency);
		tmp /= rate_;
		tmp = fabsf(tmp);

		return powf(tmp, order_);
	}

	unsigned int filter(inType* samplesIn, unsigned int numSamples, outType* samplesOut){
		unsigned int numOut = 0;

		for(unsigned int i=0; i<numSamples; i++){
			internalType sample = samplesIn[i];

			/* Run integrator stages */
			for(unsigned int j=0; j<order_; j++){
				sample = int_[j].update(sample);
			}

			outIndex_++;
			if(outIndex_ >= rate_){
				outIndex_ = 0;

				/* Run comb stages */
				for(unsigned int j=0; j<order_; j++){
					sample = comb_[j].update(sample);
				}

				samplesOut[numOut] = sample;
				numOut++;
			}
		}
		return numOut;
	}

private:
	unsigned int order_ = 0, rate_ = 1, outIndex_ = 0;

	internalType gain_;

	std::array<Integrator<internalType>, maxOrder> int_;
	std::array<Comb<internalType>, maxOrder> comb_;
};


}
}

#endif /* FILTER_CIC_CIC_H_ */
