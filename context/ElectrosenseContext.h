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


#ifndef ELECTROSENSECONTEXT_H_
#define ELECTROSENSECONTEXT_H_

#include <string>
#include <iostream>

// Default values
const std::string DEFAULT_PIPELINE = "PSD";
const int DEFAULT_LOG2_FFT_SIZE = 8;
const int DEFAULT_MONITOR_TIME  = 0;
const int DEFAULT_MIN_TIME_RES  = 0;
const int DEFAULT_DEV_INDEX = 0;
const int DEFAULT_CLK_OFF = 0;
const int DEFAULT_CLK_CORR_PERIOD = 3600;
const std::string DEFAULT_HOPPING_STRATEGY_STR = "sequential";
const float DEFAULT_GAIN = 32.8f;
const float DEFAULT_FREQ_OVERLAP = (1/6.0);
const int DEFAULT_AVG_FACTOR = 5;
const float DEFAULT_SOVERLAP  = (1<<DEFAULT_LOG2_FFT_SIZE)/2;
const std::string  DEFAULT_WINDOW_FUN_STR = "hanning";
const int DEFAULT_FFT_BATCHLEN = 10;
const int DEFAULT_CMPR_LEVEL = 6;
const int DEFAULT_SAMP_RATE = 2400000;
const std::string DEFAULT_TCP_HOSTS = "127.0.0.1:5000";
const std::string DEFAULT_TLS_HOSTS = "0";
const bool DEFAULT_FIFO_PRIORITY = true;
const int DEFAULT_BUFFER_TIME = 5; // milliseconds
const std::string DEFAULT_OUTPUT_FILENAME = "";
const unsigned long DEFAULT_START_TIME_SAMPLING = 0;

// Hopping Strategies
const int SEQUENTIAL_HOPPING_STRATEGY = 0;
const int RANDOM_HOPPING_STRATEGY     = 1;
const int SIMILARITY_HOPPING_STRATEGY = 2;

// Windows funcions
const int HANNING_WINDOW 			= 0;
const int BLACKMAN_HARRIS_WINDOW 	= 1;
const int RECTANGULAR_WINDOW	 	= 2;

//const unsigned int REF_ITERATION_TIME = 35*1000; 	// in microseconds
//const unsigned int REF_ITERATION_TIME_STEP = 450; 	// in microseconds


class ElectrosenseContext {
public:

    static ElectrosenseContext* getInstance();

    virtual ~ElectrosenseContext();

    void print();

    std::string getPipeline() const;
    void setPipeline(const std::string& pipeline);
    unsigned int getAvgFactor() const;
    void setAvgFactor(unsigned int avgFactor);
    unsigned int getClkCorrPerior() const;
    void setClkCorrPerior(unsigned int clkCorrPerior);
    int getClkOffset() const;
    void setClkOff(int clkOff);
    unsigned int getComprLevel() const;
    void setComprLevel(unsigned int comprLevel);
    int getDevIndex() const;
    void setDevIndex(int devIndex);
    unsigned int getFFTbatchlen() const;
    void setFFTbatchlen(unsigned int ffTbatchlen);
    bool isFifoPriority() const;
    void setFifoPriority(bool fifoPriority);
    float getFreqOverlap() const;
    void setFreqOverlap(float freqOverlap);
    float getGain() const;
    void setGain(float gain);
    const std::string& getHoppingStrategy() const;
    void setHoppingStrategy(const std::string& hoppingStrategy);
    unsigned int getHoppingStrategyId() const;
    unsigned int getLog2FftSize() const;
    void setLog2FftSize(unsigned int log2FftSize);
    unsigned int getReducedFFTSize();
    uint64_t getMaxFreq() const;
    void setMaxFreq(uint64_t maxFreq);
    uint64_t getMinFreq() const;
    void setMinFreq(uint64_t minFreq);

    unsigned int getMinTimeRes() const;
    void setMinTimeRes(unsigned int minTimeRes);
    unsigned int getMonitorTime() const;
    void setMonitorTime(unsigned int monitorTime);
    unsigned int getSamplingRate() const;
    void setSamplingRate(unsigned int samplingRate);
    unsigned int getSoverlap() const;
    void setSoverlap(unsigned int soverlap);
    const std::string& getTcpHosts() const;
    void setTcpHosts(const std::string& tcpHosts);
    const std::string& getTlsHosts() const;
    void setTlsHosts(const std::string& tlsHosts);
    const std::string& getWindowing() const;
    void setWindowing(const std::string& windowing);
    unsigned int getWindowingId() const;
    unsigned int getBufferTime() const;
    void setBufferTime(unsigned int bufferTime);

    std::string getOutputFileName() const;
    void setOutputFileName(const std::string& fileName);

    unsigned long getStartTimeSampling() const;
    void setStartTimeSampling (const unsigned int& startTime);

private:

    ElectrosenseContext() {
            setPipeline(DEFAULT_PIPELINE);
            setLog2FftSize(DEFAULT_LOG2_FFT_SIZE);
            setMonitorTime(DEFAULT_MONITOR_TIME);
            setMinTimeRes(DEFAULT_MIN_TIME_RES);
            setDevIndex(DEFAULT_DEV_INDEX);
            setClkOff(DEFAULT_CLK_OFF);
            setClkCorrPerior(DEFAULT_CLK_CORR_PERIOD);
            setHoppingStrategy(DEFAULT_HOPPING_STRATEGY_STR);
            setGain(DEFAULT_GAIN);
            setFreqOverlap(DEFAULT_FREQ_OVERLAP);
            setAvgFactor(DEFAULT_AVG_FACTOR);
            setSoverlap(DEFAULT_SOVERLAP);
            setWindowing(DEFAULT_WINDOW_FUN_STR);
            setFFTbatchlen(DEFAULT_FFT_BATCHLEN);
            setComprLevel(DEFAULT_CMPR_LEVEL);
            setSamplingRate(DEFAULT_SAMP_RATE);
            setTcpHosts(DEFAULT_TCP_HOSTS);
            setTlsHosts(DEFAULT_TLS_HOSTS);
            setFifoPriority(DEFAULT_FIFO_PRIORITY);
            setMinFreq(0);
            setMaxFreq(0);
            setBufferTime(DEFAULT_BUFFER_TIME);
            setOutputFileName(DEFAULT_OUTPUT_FILENAME);
            setStartTimeSampling(DEFAULT_START_TIME_SAMPLING);
    };

    static ElectrosenseContext* mInstance;


    ElectrosenseContext(ElectrosenseContext const&);
    void operator=(ElectrosenseContext const&);




//	Thread       *thread;

    uint64_t mMinFreq;
    uint64_t mMaxFreq;


    unsigned int mClkCorrPerior;
    unsigned int mSamplingRate;
    unsigned int mLog2FftSize;
    unsigned int mAvgFactor;
    unsigned int mSoverlap;
    unsigned int mMonitorTime;
    unsigned int mMinTimeRes;
    unsigned int mFFTbatchlen;
    unsigned int mComprLevel;
    int          mDevIndex;
    int          mClkOff;
    float        mGain;
    float        mFreqOverlap;
    std::string  mHoppingStrategy;
    std::string  mWindowing;
    std::string  mTcpHost;
    std::string  mTlsHost;
    std::string  mOutputFileName;
    bool 		 mFifoPriority;
    unsigned int mBufferTime;
    std::string  mPipeline;
    unsigned long mstartTimeSampling;

    unsigned int mHoppingStrategyId;
    unsigned int mWindowingId;

};

#endif /* ELECTROSENSECONTEXT_H_ */
