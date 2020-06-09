/*
 *  Created on: Oct 24, 2018
 *      Author: bertold
 */

#include <cmath>

#include "FIRDesign.h"

namespace Filter {
namespace FIR {
namespace Design {

static float besselI0(float x){
    float result = 1;
    float fac = 1;
    x = x*x/4;
    float x2 = 1;

    for(int i=1; i<=15; i++){
        fac *= i * i;
        x2 *= x;
        result += x2 / fac;
    }
    return result;
}

std::vector<float> buildKaiser(float rip, float tw, float fs){
    tw = 2*M_PI*tw/fs;

    if(rip > -21){
        rip = -21;
    }

    unsigned int m = ceilf((-7.95f-rip)/(2.285f*tw));

    float beta;
    if(rip < -50){
        beta = 0.1102f*(-8.7f-rip);
    }else{
        beta = 0.07886f*(-21.0f-rip);
        beta += 0.5842f*powf(-21.0f-rip, 0.4f);
    }


    float d = besselI0(beta);

    int windowLength = m+1;
    auto window = std::vector<float>(windowLength);

    for(unsigned int i=0; i<=m/2; i++){
        float v = besselI0(beta*sqrtf(1-powf(2*(float)i/(float)m-1,2)))/d;
        window[i] = v;
        window[m-i] = v;
    }

    return window;
}

std::vector<float> buildFIRBase(float fc, float fs, unsigned int taps, std::function<float(float)> gainFunc, float step){
	if(taps == 0){
		return std::vector<float>(0);
	}

    auto result = std::vector<float>(taps);
    float ft = fc/fs;

    if(!gainFunc){
    	if(taps == 1){
    		result[0] = 1.0f;
    		return result;
		}

        /* If gain == 1 then we can solve the fourier integral analytically */
        for(unsigned int i=0; i<taps/2; i++){
            float k = (float)i - (float)(taps-1)/2.0f;
            float p = sinf(2*M_PI*ft*k)/k/M_PI;
            result[i] = p;
            result[taps - i - 1] = p;
        }
        if(taps & 1){
            result[taps/2] = 2*ft;
        }
    }else{
    	if(taps == 1){
			result[0] = gainFunc(0);
			return result;
		}

        /* Calculate real discrete fourier transform */
        for(unsigned int i=0; i<=taps/2; i++){
            float sum = 0;
            for(float f = -ft; f <= ft; f+=step){
                float gain = gainFunc(f);
                sum += gain * cosf(2*M_PI*f*(0.5f+(float)i-(float)taps/2.0f));
            }
            sum *= step;
            result[i] = sum;
            result[taps - i - 1] = sum;
        }
    }

    return result;
}

std::vector<float> buildFIRFilter(float fc, float tw, float fs, float rip,
							      std::function<float(float)> gainFunc,
								  float step){
	unsigned int len;

	/* Build kaiser window that meets the specifications */
	auto window = buildKaiser(rip, tw, fs);
	auto filter = buildFIRBase(fc, fs, window.size(), gainFunc, step);

	/* Multiply window with filter */
	for(unsigned int i=0; i<filter.size(); i++){
		filter[i] *= window[i];
	}

	return filter;
}

}
}
}
