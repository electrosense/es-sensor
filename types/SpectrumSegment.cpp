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

#include "SpectrumSegment.h"

namespace electrosense {

    SpectrumSegment::SpectrumSegment(long sensorId,
                struct timespec timeStamp,
                long centerFrequency,
                long samplingRate,
                std::vector<std::complex<float>> samples) {

        mSensorID = sensorId;
        mTimestamp = timeStamp;
        mCenterFrequency = centerFrequency;
        mSamplingRate = samplingRate;
        mIQSamples = samples; //.swap(samples);
    }

    SpectrumSegment::~SpectrumSegment() {

        mIQSamples.clear();
        mIQSamples.shrink_to_fit();

        mPSDIQSamples.clear();
        mPSDIQSamples.shrink_to_fit();

        mPSDValues.clear();
        mPSDValues.shrink_to_fit();

    }

}