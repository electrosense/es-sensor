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
 * 	Authors: 	Roberto Calvo-Palomino <roberto.calvo@imdea.org>
 *
 */

#ifndef SRC_SENSOR_SPECTRUMSEGMENT_H_
#define SRC_SENSOR_SPECTRUMSEGMENT_H_

#include <complex.h>
#include <string.h>
#include <time.h>
#include <vector>

namespace openrfsense {

class SpectrumSegment {
  public:
    SpectrumSegment(){};

    /// \brief
    SpectrumSegment(
        long sensorId, struct timespec timeStamp, unsigned long long centerFrequency,
        long samplingRate, std::vector<std::complex<float>> samples);

    /// \brief
    SpectrumSegment(
        long sensorId, struct timespec timeStamp, unsigned long long centerFrequency,
        long samplingRate, unsigned char *samples, uint32_t samples_len);

    /// \brief
    virtual ~SpectrumSegment();

    /// \brief Returns sensors id, tipically is the MAC address of the device.
    long long getSensorId() { return mSensorID; };

    /// \brief Returns the timestamp when the segment was collected
    timespec getTimeStamp() { return mTimestamp; };

    /// \brief Returns the center frequency of the RF receiver
    unsigned long long getCenterFrequency() { return mCenterFrequency; };

    /// \brief Returns the sampling rate of the RF receiver for this segment.
    long getSamplingRate() { return mSamplingRate; };

    /// \brief Returns original raw data collected in the RTL-SDR ( I and Q use
    /// 8-bit)
    unsigned char *get_buffer() { return mSamples; };
    unsigned int get_buffer_len() { return mSamples_len; };

    /// \brief Returns I/Q data in time domain ( I and Q use 32-bits)
    std::vector<std::complex<float>> &getIQSamples() { return mIQSamples; };

    /// \brief Returns I/Q data in the frequency domain ( I and Q use 32-bits)
    std::vector<std::complex<float>> &getIQSamplesFreq() { return mIQSamplesFreq; };

    /// \brief Returns PSD (Power Spectral Density) of this segment.
    std::vector<float> &getPSDValues() { return mPSDValues; };

    /// \brief Sets the AVRO buffer and its size
    void setAvroBuffer(char *buffer, unsigned int size) {
        mAvroBuffer = buffer;
        mAvroSize = size;
    };

    /// \brief Returns the AVRO buffer
    char *getAvroBuffer() { return mAvroBuffer; };

    /// \brief Returns the AVRO buffer size
    unsigned int getAvroBufferSize() { return mAvroSize; };

    /// \brief Returns I/Q data in time domain ( I and Q use 32-bits) v2
    std::vector<float> &getIQ_time_v2() { return mIQ_time_v2; };

  private:
    long long mSensorID;
    struct timespec mTimestamp;
    unsigned long long mCenterFrequency;
    long mSamplingRate;

    std::vector<std::complex<float>> mIQSamples;
    std::vector<std::complex<float>> mIQSamplesFreq;
    std::vector<float> mPSDValues;

    std::vector<float> mIQ_time_v2;

    char *mAvroBuffer;
    unsigned int mAvroSize;

    unsigned char *mSamples;
    uint32_t mSamples_len;
};
} // namespace openrfsense
#endif /* SRC_SENSOR_SPECTRUMSEGMENT_H_ */
