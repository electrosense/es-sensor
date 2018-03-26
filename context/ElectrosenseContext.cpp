/*
 * Copyright (C) 2017 by IMDEA Networks Institute
 *
 * This file is part of Electrosense.
 *
 * Electrosense is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Electrosense is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RTL-Spec.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 	Authors:
 * 	    Roberto Calvo Palomino <roberto.calvo@imdea.org>
 *
 */


#include "ElectrosenseContext.h"

ElectrosenseContext* ElectrosenseContext::mInstance = NULL;

ElectrosenseContext* ElectrosenseContext::getInstance()
{
    if (mInstance == NULL)
    {
        mInstance = new ElectrosenseContext();
    }

    return mInstance;
}


ElectrosenseContext::~ElectrosenseContext() {

}

void ElectrosenseContext::print()
{

    std::cout << "Min Freq: " << getMinFreq() << std::endl;
    std::cout << "Max Freq: " << getMaxFreq() << std::endl;
    std::cout << "Windowing: " << getWindowing() << std::endl;
    std::cout << "AvgFactor: " << getAvgFactor() << std::endl;
    std::cout << "SamplingRate: " << getSamplingRate() << std::endl;
    std::cout << "HoppingStrategy: " << getHoppingStrategy() << std::endl;
    std::cout << "Gain: " << getGain() << std::endl;
    std::cout << "Priority: " << isFifoPriority() << std::endl;

    std::cout << "log2FFTSize: " <<  getLog2FftSize() << std::endl;
    std::cout << "FFT batch len: " << getFFTbatchlen() << std::endl;
    std::cout << "Monitor Time: " << getMonitorTime() << std::endl;
    std::cout << "MinTimeRes: " << getMinTimeRes() << std::endl;
    std::cout << "Device Index: " << getDevIndex() << std::endl;
    std::cout << "Clk Offset: " << getClkOffset() << std::endl;
    std::cout << "Clk Corr Period: " << getClkCorrPerior() << std::endl;
    std::cout << "Freq Overlap: " << getFreqOverlap()  << std::endl;
    std::cout << "Soverlap: " << getSoverlap() << std::endl;
    std::cout << "Compresion Level: " << getComprLevel()  << std::endl;
    std::cout << "TcpHosts: " << getTcpHosts() << std::endl;
#if defined(RPI_GPU)
    std::cout << "FFT Computation: " << "GPU" << std::endl;
#else
    std::cout << "FFT Computation: " << "CPU" << std::endl;
#endif

}
unsigned int ElectrosenseContext::getAvgFactor() const {
    return mAvgFactor;
}

void ElectrosenseContext::setAvgFactor(unsigned int avgFactor) {

    if(avgFactor < 1)
        mAvgFactor = DEFAULT_AVG_FACTOR;
    else
        mAvgFactor = avgFactor;
}

unsigned int ElectrosenseContext::getClkCorrPerior() const {
    return mClkCorrPerior;
}

void ElectrosenseContext::setClkCorrPerior(unsigned int clkCorrPerior) {
    mClkCorrPerior = clkCorrPerior;
}

int ElectrosenseContext::getClkOffset() const {
    return mClkOff;
}

void ElectrosenseContext::setClkOff(int clkOff) {
    mClkOff = clkOff;
}

unsigned int ElectrosenseContext::getComprLevel() const {
    return mComprLevel;
}

void ElectrosenseContext::setComprLevel(unsigned int comprLevel) {

    if(comprLevel > 9)
        comprLevel = DEFAULT_CMPR_LEVEL;

    mComprLevel = comprLevel;
}

int ElectrosenseContext::getDevIndex() const {
    return mDevIndex;
}

void ElectrosenseContext::setDevIndex(int devIndex) {
    mDevIndex = devIndex;
}

unsigned int ElectrosenseContext::getFFTbatchlen() const {
    return mFFTbatchlen;
}

void ElectrosenseContext::setFFTbatchlen(unsigned int ffTbatchlen) {
    mFFTbatchlen = ffTbatchlen;
}

bool ElectrosenseContext::isFifoPriority() const {
    return mFifoPriority;
}

void ElectrosenseContext::setFifoPriority(bool fifoPriority) {
    mFifoPriority = fifoPriority;
}

std::string ElectrosenseContext::getOutputFileName() const {
    return mOutputFileName;
}

void ElectrosenseContext::setOutputFileName(const std::string& fileName) {
    mOutputFileName = fileName;
}


float ElectrosenseContext::getFreqOverlap() const {
    return mFreqOverlap;
}

void ElectrosenseContext::setFreqOverlap(float freqOverlap) {
    mFreqOverlap = freqOverlap;
}

float ElectrosenseContext::getGain() const {
    return mGain;
}

void ElectrosenseContext::setGain(float gain) {
    mGain = gain;
}

unsigned int ElectrosenseContext::getReducedFFTSize()
{
    uint32_t fft_size = 1 << ElectrosenseContext::getInstance()->getLog2FftSize();
    uint32_t reduced_fft_size =  (1-ElectrosenseContext::getInstance()->getFreqOverlap())*(fft_size+1);

    // We need an odd number of bins
    if (reduced_fft_size % 2 == 0)
        reduced_fft_size++;

    return reduced_fft_size;

}

const std::string& ElectrosenseContext::getHoppingStrategy() const {
    return mHoppingStrategy;
}

void ElectrosenseContext::setHoppingStrategy(const std::string& hoppingStrategy)
{
    mHoppingStrategy = hoppingStrategy;
    if (mHoppingStrategy.compare("sequential") == 0)
        mHoppingStrategyId = SEQUENTIAL_HOPPING_STRATEGY;
    else if (mHoppingStrategy.compare("random") == 0)
        mHoppingStrategyId = RANDOM_HOPPING_STRATEGY;
    else if (mHoppingStrategy.compare("similarity") == 0)
        mHoppingStrategyId = SIMILARITY_HOPPING_STRATEGY;
}

unsigned int ElectrosenseContext::getHoppingStrategyId() const
{
    return mHoppingStrategyId;
}
unsigned int ElectrosenseContext::getLog2FftSize() const {
    return mLog2FftSize;
}

void ElectrosenseContext::setLog2FftSize(unsigned int log2FftSize) {
    mLog2FftSize = log2FftSize;
}

unsigned int ElectrosenseContext::getMaxFreq() const {
    return mMaxFreq;
}

void ElectrosenseContext::setMaxFreq(unsigned int maxFreq) {
    mMaxFreq = maxFreq;
}

unsigned int ElectrosenseContext::getMinFreq() const {
    return mMinFreq;
}

void ElectrosenseContext::setMinFreq(unsigned int minFreq) {
    mMinFreq = minFreq;
}

unsigned int ElectrosenseContext::getMinTimeRes() const {
    return mMinTimeRes;
}

void ElectrosenseContext::setMinTimeRes(unsigned int minTimeRes) {
    mMinTimeRes = minTimeRes;
}

unsigned int ElectrosenseContext::getMonitorTime() const {
    return mMonitorTime;
}

void ElectrosenseContext::setMonitorTime(unsigned int monitorTime) {
    mMonitorTime = monitorTime;
}

unsigned int ElectrosenseContext::getSamplingRate() const {
    return mSamplingRate;
}

void ElectrosenseContext::setSamplingRate(unsigned int samplingRate) {
    mSamplingRate = samplingRate;
}

unsigned int ElectrosenseContext::getSoverlap() const {
    return mSoverlap;
}

void ElectrosenseContext::setSoverlap(unsigned int soverlap) {

    if(soverlap > (1<< getLog2FftSize())-1)
        mSoverlap = (1<<getLog2FftSize())/2;
    else
        mSoverlap = soverlap;
}


const std::string& ElectrosenseContext::getTlsHosts() const {
    return mTlsHost;
}

void ElectrosenseContext::setTlsHosts(const std::string& tlsHosts) {
    mTlsHost = tlsHosts;
}

const std::string& ElectrosenseContext::getTcpHosts() const {
    return mTcpHost;
}

void ElectrosenseContext::setTcpHosts(const std::string& tcpHosts) {
    mTcpHost = tcpHosts;
}

const std::string& ElectrosenseContext::getWindowing() const {
    return mWindowing;
}

void ElectrosenseContext::setWindowing(const std::string& windowing) {
    mWindowing = windowing;

    if (mWindowing.compare("hanning") == 0)
        mWindowingId = HANNING_WINDOW;
    else if (mWindowing.compare("blackman_harris_4") == 0)
        mWindowingId = BLACKMAN_HARRIS_WINDOW;
    else
        mWindowingId = RECTANGULAR_WINDOW;

}

unsigned int ElectrosenseContext::getWindowingId() const
{
    return mWindowingId;
}

unsigned int ElectrosenseContext::getBufferTime() const
{
    return mBufferTime;
}

void ElectrosenseContext::setBufferTime(unsigned int bufferTime)
{
    mBufferTime = bufferTime;
}
