
#include <iostream>

#include "Filter/CIC/CIC.h"
#include "Filter/FIR/FIRDesign.h"
#include "Filter/FIR/RationalFIRFilter.h"
#include "Filter/Resampler/Downsampler.h"

//int demo_float(){
//	Filter::Resampler::ResamplerConfig cfg;
//	cfg.inputRate = 2.4e6;
//	cfg.outputRate = 60e3;
//	cfg.transitionWidthFraction = 0.2f;
//	Filter::Resampler::Downsampler<int8_t, float> ri(cfg);
//	Filter::Resampler::Downsampler<int8_t, float> rq(cfg);
//
//	std::cout << "Final output rate: " <<ri.getActualOutputRate()<<std::endl;
//
//	unsigned int numSamples = 512*50;
//
//	int8_t* inputVectorI = (int8_t*)malloc(numSamples * sizeof(int8_t));
//	int8_t* inputVectorQ = (int8_t*)malloc(numSamples * sizeof(int8_t));
//	float* outputVectorI = (float*)malloc(numSamples * sizeof(float));
//	float* outputVectorQ = (float*)malloc(numSamples * sizeof(float));
//
//	for(float f = 0; f<4; f+=0.01){
//		float freq = f*cfg.outputRate;
//
//		for(unsigned int i=0; i<numSamples; i++){
//			inputVectorI[i] = 127*cosf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
//			inputVectorQ[i] = 127*sinf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
//			//std::cout << inputVectorI[i]<< " " << inputVectorQ[i]<<std::endl;
//		}
//
//		unsigned int outputSamples = 0;
//		auto t = ri.filter(inputVectorI, numSamples, outputVectorI+outputSamples);
//		rq.filter(inputVectorQ, numSamples, outputVectorQ+outputSamples);
//		outputSamples += t;
//
//		//std::cout << "Made " << outputSamples << "samples" << std::endl;
//
//		float sum = 0;
//		for(unsigned int i=100; i<outputSamples; i++){
//			float a = outputVectorI[i]*outputVectorI[i];
//			a += outputVectorQ[i]*outputVectorQ[i];
//			a = sqrtf(a);
//			sum += a;
//		}
//
//		sum /= (outputSamples-100);
//
//		sum = 20*log10f(sum);
//
//		std::cout << f<< ' ' << sum << std::endl;
//	}
//
//	return 0;
//}
//
//int demo_int8_t(){
//	Filter::Resampler::ResamplerConfig cfg;
//	cfg.inputRate = 2.4e6;
//	cfg.outputRate = 60e3;
//	cfg.transitionWidthFraction = 0.2f;
//	cfg.desiredRipple = -40; /* WARNING: IN 8-bit THERE IS NO POINT IN GOING LOWER AS THE SAMPLES WILL BE 0 */
//	Filter::Resampler::Downsampler<int8_t, int8_t> ri(cfg);
//	Filter::Resampler::Downsampler<int8_t, int8_t> rq(cfg);
//
//	std::cout << "Final output rate: " <<ri.getActualOutputRate()<<std::endl;
//
//	unsigned int numSamples = 512*50;
//
//	int8_t* inputVectorI = (int8_t*)malloc(numSamples * sizeof(int8_t));
//	int8_t* inputVectorQ = (int8_t*)malloc(numSamples * sizeof(int8_t));
//	int8_t* outputVectorI = (int8_t*)malloc(numSamples * sizeof(int8_t));
//	int8_t* outputVectorQ = (int8_t*)malloc(numSamples * sizeof(int8_t));
//
//	for(float f = 0; f<4; f+=0.01){
//		float freq = f*cfg.outputRate;
//
//		for(unsigned int i=0; i<numSamples; i++){
//			inputVectorI[i] = 127*cosf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
//			inputVectorQ[i] = 127*sinf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
//			//std::cout << inputVectorI[i]<< " " << inputVectorQ[i]<<std::endl;
//		}
//
//		unsigned int outputSamples = 0;
//		auto t = ri.filter(inputVectorI, numSamples, outputVectorI+outputSamples);
//		rq.filter(inputVectorQ, numSamples, outputVectorQ+outputSamples);
//		outputSamples += t;
//
//		//std::cout << "Made " << outputSamples << "samples" << std::endl;
//
//		float sum = 0;
//		for(unsigned int i=100; i<outputSamples; i++){
//			float a = (float)outputVectorI[i]*(float)outputVectorI[i];
//			a += (float)outputVectorQ[i]*(float)outputVectorQ[i];
//			a = sqrtf(a);
//			sum += a;
//		}
//
//		sum /= (outputSamples-100);
//
//		sum = 20*log10f(sum);
//
//		std::cout << f<< ' ' << sum << std::endl;
//	}
//
//	return 0;
//}
//
//int demo_uint8_t(){
//	Filter::Resampler::ResamplerConfig cfg;
//	cfg.inputRate = 2.4e6;
//	cfg.outputRate = 60e3;
//	cfg.transitionWidthFraction = 0.2f;
//	cfg.desiredRipple = -40; /* WARNING: IN 8-bit THERE IS NO POINT IN GOING LOWER AS THE SAMPLES WILL BE 0 */
//	/* WARNING WHEN USING UNSIGNED INPUT YOU NEED TO CHANGE THE CIC TYPE TO UINT32_T!!!, SEE NEXT LINE */
//	Filter::Resampler::Downsampler<uint8_t, uint8_t, true, uint32_t> ri(cfg);
//	Filter::Resampler::Downsampler<uint8_t, uint8_t, true, uint32_t> rq(cfg);
//
//	std::cout << "Final output rate: " <<ri.getActualOutputRate()<<std::endl;
//
//	unsigned int numSamples = 512*50;
//
//	uint8_t* inputVectorI = (uint8_t*)malloc(numSamples * sizeof(uint8_t));
//	uint8_t* inputVectorQ = (uint8_t*)malloc(numSamples * sizeof(uint8_t));
//	uint8_t* outputVectorI = (uint8_t*)malloc(numSamples * sizeof(uint8_t));
//	uint8_t* outputVectorQ = (uint8_t*)malloc(numSamples * sizeof(uint8_t));
//
//	for(float f = 0; f<4; f+=0.01){
//		float freq = f*cfg.outputRate;
//
//		for(unsigned int i=0; i<numSamples; i++){
//			inputVectorI[i] = 128+127*cosf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
//			inputVectorQ[i] = 128+127*sinf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
//			//std::cout << inputVectorI[i]<< " " << inputVectorQ[i]<<std::endl;
//		}
//
//		unsigned int outputSamples = 0;
//		auto t = ri.filter(inputVectorI, numSamples, outputVectorI+outputSamples);
//		rq.filter(inputVectorQ, numSamples, outputVectorQ+outputSamples);
//		outputSamples += t;
//
//		//std::cout << "Made " << outputSamples << "samples" << std::endl;
//
//
//		float sum = 0;
//		for(unsigned int i=100; i<outputSamples; i++){
//			float a = (float)(outputVectorI[i]-128)*(float)(outputVectorI[i]-128);
//			a += (float)(outputVectorQ[i]-128)*(float)(outputVectorQ[i]-128);
//			a = sqrtf(a);
//			sum += a;
//		}
//
//		sum /= (outputSamples-100);
//
//		sum = 20*log10f(sum);
//
//		std::cout << f<< ' ' << sum << std::endl;
//	}
//
//	return 0;
//}


int demo_float(){
	Filter::Resampler::ResamplerConfig cfg;
	cfg.rational = false;
	cfg.inputRate = 2.4e6;
	cfg.outputRate = 61.123e3;
	cfg.transitionWidthFraction = 0.2f;
	Filter::Resampler::Downsampler<int8_t, float> ri(cfg);
	Filter::Resampler::Downsampler<int8_t, float> rq(cfg);

	std::cout << "Final output rate: " <<ri.getActualOutputRate()<<std::endl;

	unsigned int numSamples = 512*50;

	int8_t* inputVectorI = (int8_t*)malloc(numSamples * sizeof(int8_t));
	int8_t* inputVectorQ = (int8_t*)malloc(numSamples * sizeof(int8_t));
	float* outputVectorI = (float*)malloc(numSamples * sizeof(float));
	float* outputVectorQ = (float*)malloc(numSamples * sizeof(float));

	for(float f = 0; f<4; f+=0.01){
		float freq = f*cfg.outputRate;

		for(unsigned int i=0; i<numSamples; i++){
			inputVectorI[i] = 127*cosf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
			inputVectorQ[i] = 127*sinf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
			//std::cout << inputVectorI[i]<< " " << inputVectorQ[i]<<std::endl;
		}

		unsigned int outputSamples = 0;
		auto t = ri.filter(inputVectorI, numSamples, outputVectorI+outputSamples);
		rq.filter(inputVectorQ, numSamples, outputVectorQ+outputSamples);
		outputSamples += t;



		float sum = 0;
		for(unsigned int i=100; i<outputSamples; i++){
			float a = outputVectorI[i]*outputVectorI[i];
			a += outputVectorQ[i]*outputVectorQ[i];
			a = sqrtf(a);
			sum += a;
		}

		sum /= (outputSamples-100);

		sum = 20*log10f(sum);

		std::cout << f<< ' ' << sum << std::endl;
	}

	return 0;
}

int demo_int8_t(){
	Filter::Resampler::ResamplerConfig cfg;
	cfg.rational = false;
	cfg.inputRate = 2.4e6;
	cfg.outputRate = 61.123e3;
	cfg.transitionWidthFraction = 0.2f;
	cfg.desiredRipple = -40; /* WARNING: IN 8-bit THERE IS NO POINT IN GOING LOWER AS THE SAMPLES WILL BE 0 */
	Filter::Resampler::Downsampler<int8_t, int8_t> ri(cfg);
	Filter::Resampler::Downsampler<int8_t, int8_t> rq(cfg);

	std::cout << "Final output rate: " <<ri.getActualOutputRate()<<std::endl;

	unsigned int numSamples = 512*50;

	int8_t* inputVectorI = (int8_t*)malloc(numSamples * sizeof(int8_t));
	int8_t* inputVectorQ = (int8_t*)malloc(numSamples * sizeof(int8_t));
	int8_t* outputVectorI = (int8_t*)malloc(numSamples * sizeof(int8_t));
	int8_t* outputVectorQ = (int8_t*)malloc(numSamples * sizeof(int8_t));

	for(float f = 0; f<4; f+=0.01){
		float freq = f*cfg.outputRate;

		for(unsigned int i=0; i<numSamples; i++){
			inputVectorI[i] = 127*cosf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
			inputVectorQ[i] = 127*sinf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
			//std::cout << inputVectorI[i]<< " " << inputVectorQ[i]<<std::endl;
		}

		unsigned int outputSamples = 0;
		auto t = ri.filter(inputVectorI, numSamples, outputVectorI+outputSamples);
		rq.filter(inputVectorQ, numSamples, outputVectorQ+outputSamples);
		outputSamples += t;

		//std::cout << "Made " << outputSamples << "samples" << std::endl;

		float sum = 0;
		for(unsigned int i=100; i<outputSamples; i++){
			float a = (float)outputVectorI[i]*(float)outputVectorI[i];
			a += (float)outputVectorQ[i]*(float)outputVectorQ[i];
			a = sqrtf(a);
			sum += a;
		}

		sum /= (outputSamples-100);

		sum = 20*log10f(sum);

		std::cout << f<< ' ' << sum << std::endl;
	}

	return 0;
}

int demo_uint8_t(){
	Filter::Resampler::ResamplerConfig cfg;
	cfg.rational = false;
	cfg.inputRate = 2.4e6;
	cfg.outputRate = 61.123e3;
	cfg.transitionWidthFraction = 0.2f;
	cfg.desiredRipple = -40; /* WARNING: IN 8-bit THERE IS NO POINT IN GOING LOWER AS THE SAMPLES WILL BE 0 */
	/* WARNING WHEN USING UNSIGNED INPUT YOU NEED TO CHANGE THE CIC TYPE TO UINT32_T!!!, SEE NEXT LINE */
	Filter::Resampler::Downsampler<uint8_t, uint8_t, uint32_t> ri(cfg);
	Filter::Resampler::Downsampler<uint8_t, uint8_t, uint32_t> rq(cfg);

	std::cout << "Final output rate: " <<ri.getActualOutputRate()<<std::endl;

	unsigned int numSamples = 512*50;

	uint8_t* inputVectorI = (uint8_t*)malloc(numSamples * sizeof(uint8_t));
	uint8_t* inputVectorQ = (uint8_t*)malloc(numSamples * sizeof(uint8_t));
	uint8_t* outputVectorI = (uint8_t*)malloc(numSamples * sizeof(uint8_t));
	uint8_t* outputVectorQ = (uint8_t*)malloc(numSamples * sizeof(uint8_t));

	for(float f = 0; f<4; f+=0.01){
		float freq = f*cfg.outputRate;

		for(unsigned int i=0; i<numSamples; i++){
			inputVectorI[i] = 128+127*cosf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
			inputVectorQ[i] = 128+127*sinf(2.0f*M_PI*(float)i*freq/cfg.inputRate);
			//std::cout << inputVectorI[i]<< " " << inputVectorQ[i]<<std::endl;
		}

		unsigned int outputSamples = 0;
		auto t = ri.filter(inputVectorI, numSamples, outputVectorI+outputSamples);
		rq.filter(inputVectorQ, numSamples, outputVectorQ+outputSamples);
		outputSamples += t;

		//std::cout << "Made " << outputSamples << "samples" << std::endl;


		float sum = 0;
		for(unsigned int i=100; i<outputSamples; i++){
			float a = (float)(outputVectorI[i]-128)*(float)(outputVectorI[i]-128);
			a += (float)(outputVectorQ[i]-128)*(float)(outputVectorQ[i]-128);
			a = sqrtf(a);
			sum += a;
		}

		sum /= (outputSamples-100);

		sum = 20*log10f(sum);

		std::cout << f<< ' ' << sum << std::endl;
	}

	return 0;
}

int main(){
	demo_int8_t();
	return 0;
}

