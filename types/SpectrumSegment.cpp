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

#include "SpectrumSegment.h"

namespace openrfsense {

SpectrumSegment::SpectrumSegment(
    long sensorId, struct timespec timeStamp, unsigned long long centerFrequency,
    long samplingRate, std::vector<std::complex<float>> samples) {

    mSensorID = sensorId;
    mTimestamp = timeStamp;
    mCenterFrequency = centerFrequency;
    mSamplingRate = samplingRate;
    mIQSamples = samples;
    mAvroBuffer = NULL;
    mSamples = NULL;
}

SpectrumSegment::SpectrumSegment(
    long sensorId, struct timespec timeStamp, unsigned long long centerFrequency,
    long samplingRate, unsigned char *samples, uint32_t samples_len) {

    mSensorID = sensorId;
    mTimestamp = timeStamp;
    mCenterFrequency = centerFrequency;
    mSamplingRate = samplingRate;

    mSamples_len = samples_len;

    mSamples = (unsigned char *)malloc(sizeof(unsigned char) * samples_len);
    memcpy(mSamples, samples, sizeof(unsigned char) * samples_len);

    mAvroBuffer = NULL;
}

SpectrumSegment::~SpectrumSegment() {

    mIQSamples.clear();
    mIQSamples.shrink_to_fit();

    mIQSamplesFreq.clear();
    mIQSamplesFreq.shrink_to_fit();

    mPSDValues.clear();
    mPSDValues.shrink_to_fit();

    mIQ_time_v2.clear();
    mIQ_time_v2.shrink_to_fit();

    if (mSamples)
        free(mSamples);

    if (mAvroBuffer)
        free(mAvroBuffer);
}

} // namespace openrfsense
