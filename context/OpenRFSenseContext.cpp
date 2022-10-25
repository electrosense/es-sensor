/*
 * Copyright (C) 2018 by IMDEA Networks Institute
 *
 * This file is part of Electrosense.
 *
 * Electrosense is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
 * 	    Roberto Calvo-Palomino <roberto.calvo@imdea.org>
 *
 */

#include "OpenRFSenseContext.h"

OpenRFSenseContext *OpenRFSenseContext::mInstance = NULL;

OpenRFSenseContext *OpenRFSenseContext::getInstance() {
    if (mInstance == NULL) {
        mInstance = new OpenRFSenseContext();
    }

    return mInstance;
}

OpenRFSenseContext::~OpenRFSenseContext() {}

void OpenRFSenseContext::print() {
    std::cout << "---- Config ---- " << std::endl;
    std::cout << "Min Freq: " << getMinFreq() << std::endl;
    std::cout << "Max Freq: " << getMaxFreq() << std::endl;
    std::cout << "Pipeline: " << getPipeline() << std::endl;
    std::cout << "Windowing: " << getWindowing() << std::endl;
    std::cout << "AvgFactor: " << getAvgFactor() << std::endl;
    std::cout << "SamplingRate: " << getSamplingRate() << std::endl;
    std::cout << "HoppingStrategy: " << getHoppingStrategy() << std::endl;
    std::cout << "Gain: " << getGain() << std::endl;
    std::cout << "Priority: " << isFifoPriority() << std::endl;
    std::cout << "Direct Sampling Mode: " << getDirectSamplingMode() << std::endl;

    std::cout << "log2FFTSize: " << getLog2FftSize() << std::endl;
    std::cout << "FFT batch len: " << getFFTbatchlen() << std::endl;
    std::cout << "Monitor Time: " << getMonitorTime() << std::endl;
    std::cout << "MinTimeRes: " << getMinTimeRes() << std::endl;
    std::cout << "Device Index: " << getDevIndex() << std::endl;
    std::cout << "Clk Offset: " << getClkOffset() << std::endl;
    std::cout << "Clk Corr Period: " << getClkCorrPerior() << std::endl;
    std::cout << "Freq Overlap: " << getFreqOverlap() << std::endl;
    std::cout << "Soverlap: " << getSoverlap() << std::endl;
    std::cout << "Compresion Level: " << getComprLevel() << std::endl;
    std::cout << "TcpHosts: " << getTcpHosts() << std::endl;
#if defined(RPI_GPU)
    std::cout << "FFT Computation: "
              << "GPU" << std::endl;
#else
    std::cout << "FFT Computation: "
              << "CPU" << std::endl;
#endif

    std::cout << "Absolute Start Time (IQ): " << getStartTimeSampling() << std::endl;

    if (!getOutputFileName().empty()) {
        std::cout << "Output file: " << getOutputFileName() << std::endl;
        std::cout << "Output type: " << (mOutputType ? "FLOAT" : "BYTE") << std::endl;
    }

    std::cout << "---- End Config ---- " << std::endl << std::endl;
}

std::string OpenRFSenseContext::getSensorId() const { return mSensorId; }

void OpenRFSenseContext::setSensorId(const std::string &id) {
    mSensorId = id;
}

std::string OpenRFSenseContext::getPipeline() const { return mPipeline; }

void OpenRFSenseContext::setPipeline(const std::string &pipeline) {
    mPipeline = pipeline;
}
unsigned int OpenRFSenseContext::getAvgFactor() const { return mAvgFactor; }

void OpenRFSenseContext::setAvgFactor(unsigned int avgFactor) {

    if (avgFactor < 1)
        mAvgFactor = DEFAULT_AVG_FACTOR;
    else
        mAvgFactor = avgFactor;
}

unsigned int OpenRFSenseContext::getClkCorrPerior() const { return mClkCorrPerior; }

void OpenRFSenseContext::setClkCorrPerior(unsigned int clkCorrPerior) {
    mClkCorrPerior = clkCorrPerior;
}

int OpenRFSenseContext::getClkOffset() const { return mClkOff; }

void OpenRFSenseContext::setClkOff(int clkOff) { mClkOff = clkOff; }

unsigned int OpenRFSenseContext::getComprLevel() const { return mComprLevel; }

void OpenRFSenseContext::setComprLevel(unsigned int comprLevel) {
    if (comprLevel > 9)
        comprLevel = DEFAULT_CMPR_LEVEL;

    mComprLevel = comprLevel;
}

int OpenRFSenseContext::getDevIndex() const { return mDevIndex; }

void OpenRFSenseContext::setDevIndex(int devIndex) { mDevIndex = devIndex; }

unsigned int OpenRFSenseContext::getFFTbatchlen() const { return mFFTbatchlen; }

void OpenRFSenseContext::setFFTbatchlen(unsigned int ffTbatchlen) {
    mFFTbatchlen = ffTbatchlen;
}

bool OpenRFSenseContext::isFifoPriority() const { return mFifoPriority; }

void OpenRFSenseContext::setFifoPriority(bool fifoPriority) {
    mFifoPriority = fifoPriority;
}

int OpenRFSenseContext::getDirectSamplingMode() const { return mDirectSamplingMode; }

void OpenRFSenseContext::setDirectSamplingMode(int directSamplingMode) {
    mDirectSamplingMode = directSamplingMode;
}

std::string OpenRFSenseContext::getOutputFileName() const { return mOutputFileName; }

template <typename T>
inline std::vector<std::string>
OpenRFSenseContext::split(const std::string &s, T delimiter) {
    std::vector<std::string> result;

    std::size_t current = 0;
    std::size_t p = s.find_first_of(delimiter, 0);

    while (p != std::string::npos) {
        result.emplace_back(s, current, p - current);
        current = p + 1;
        p = s.find_first_of(delimiter, current);
    }

    result.emplace_back(s, current);

    return result;
}

void OpenRFSenseContext::setOutputFileName(const std::string &fileName) {

    if (fileName.empty())
        return;

    std::vector<std::string> slices = split(fileName, ":");

    if (slices.size() > 1) {
        if (slices[0].compare("BYTE") == 0)
            mOutputType = OUTPUT_TYPE_BYTE;
        else if (slices[0].compare("FLOAT") == 0)
            mOutputType = OUTPUT_TYPE_FLOAT;
        else {
            std::cout << "Output format not recognized, use only BYTE or FLOAT"
                      << std::endl;
            exit(-1);
        }
        mOutputFileName = slices[1];
    } else {
        mOutputType = OUTPUT_TYPE_BYTE;
        mOutputFileName = fileName;
    }
}

unsigned long OpenRFSenseContext::getStartTimeSampling() const {
    return mstartTimeSampling;
}

void OpenRFSenseContext::setStartTimeSampling(const unsigned int &startTime) {
    mstartTimeSampling = startTime;
}

float OpenRFSenseContext::getFreqOverlap() const { return mFreqOverlap; }

void OpenRFSenseContext::setFreqOverlap(float freqOverlap) {
    mFreqOverlap = freqOverlap;
}

float OpenRFSenseContext::getGain() const { return mGain; }

void OpenRFSenseContext::setGain(float gain) { mGain = gain; }

unsigned int OpenRFSenseContext::getReducedFFTSize() {
    uint32_t fft_size = 1 << OpenRFSenseContext::getInstance()->getLog2FftSize();
    uint32_t reduced_fft_size =
        (1 - OpenRFSenseContext::getInstance()->getFreqOverlap()) * (fft_size + 1);

    // We need an odd number of bins
    if (reduced_fft_size % 2 == 0)
        reduced_fft_size++;

    return reduced_fft_size;
}

const std::string &OpenRFSenseContext::getHoppingStrategy() const {
    return mHoppingStrategy;
}

void OpenRFSenseContext::setHoppingStrategy(const std::string &hoppingStrategy) {
    mHoppingStrategy = hoppingStrategy;
    if (mHoppingStrategy.compare("sequential") == 0)
        mHoppingStrategyId = SEQUENTIAL_HOPPING_STRATEGY;
    else if (mHoppingStrategy.compare("random") == 0)
        mHoppingStrategyId = RANDOM_HOPPING_STRATEGY;
    else if (mHoppingStrategy.compare("similarity") == 0)
        mHoppingStrategyId = SIMILARITY_HOPPING_STRATEGY;
}

unsigned int OpenRFSenseContext::getHoppingStrategyId() const {
    return mHoppingStrategyId;
}
unsigned int OpenRFSenseContext::getLog2FftSize() const { return mLog2FftSize; }

void OpenRFSenseContext::setLog2FftSize(unsigned int log2FftSize) {
    mLog2FftSize = log2FftSize;
}

uint64_t OpenRFSenseContext::getMaxFreq() const { return mMaxFreq; }

void OpenRFSenseContext::setMaxFreq(uint64_t maxFreq) { mMaxFreq = maxFreq; }

uint64_t OpenRFSenseContext::getMinFreq() const { return mMinFreq; }

void OpenRFSenseContext::setMinFreq(uint64_t minFreq) { mMinFreq = minFreq; }

unsigned int OpenRFSenseContext::getMinTimeRes() const { return mMinTimeRes; }

void OpenRFSenseContext::setMinTimeRes(unsigned int minTimeRes) {
    mMinTimeRes = minTimeRes;
}

unsigned int OpenRFSenseContext::getMonitorTime() const { return mMonitorTime; }

void OpenRFSenseContext::setMonitorTime(unsigned int monitorTime) {
    mMonitorTime = monitorTime;
}

unsigned int OpenRFSenseContext::getSamplingRate() const { return mSamplingRate; }

void OpenRFSenseContext::setSamplingRate(unsigned int samplingRate) {
    mSamplingRate = samplingRate;
}

unsigned int OpenRFSenseContext::getSoverlap() const { return mSoverlap; }

void OpenRFSenseContext::setSoverlap(unsigned int soverlap) {

    if (soverlap > (unsigned int)(1 << getLog2FftSize()) - 1)
        mSoverlap = (1 << getLog2FftSize()) / 2;
    else
        mSoverlap = soverlap;
}

const std::string &OpenRFSenseContext::getTlsHosts() const { return mTlsHost; }

void OpenRFSenseContext::setTlsHosts(const std::string &tlsHosts) {
    mTlsHost = tlsHosts;
}

const std::string &OpenRFSenseContext::getTcpHosts() const { return mTcpHost; }

void OpenRFSenseContext::setTcpHosts(const std::string &tcpHosts) {
    mTcpHost = tcpHosts;
}

const std::string &OpenRFSenseContext::getWindowing() const { return mWindowing; }

void OpenRFSenseContext::setWindowing(const std::string &windowing) {
    mWindowing = windowing;

    if (mWindowing.compare("hanning") == 0)
        mWindowingId = HANNING_WINDOW;
    else if (mWindowing.compare("blackman_harris_4") == 0)
        mWindowingId = BLACKMAN_HARRIS_WINDOW;
    else
        mWindowingId = RECTANGULAR_WINDOW;
}

unsigned int OpenRFSenseContext::getWindowingId() const { return mWindowingId; }

unsigned int OpenRFSenseContext::getBufferTime() const { return mBufferTime; }

void OpenRFSenseContext::setBufferTime(unsigned int bufferTime) {
    mBufferTime = bufferTime;
}
