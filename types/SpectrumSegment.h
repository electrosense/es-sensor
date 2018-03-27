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

#ifndef SRC_SENSOR_SPECTRUMSEGMENT_H_
#define SRC_SENSOR_SPECTRUMSEGMENT_H_

#include <vector>
#include <time.h>
#include <complex.h>

namespace electrosense {

	class SpectrumSegment {
	public:

		SpectrumSegment() {};

		SpectrumSegment(long sensorId,
						struct timespec timeStamp,
						unsigned long long centerFrequency,
						long samplingRate,
						std::vector<std::complex<float>> samples);

		virtual ~SpectrumSegment();

		long long getSensorId() { return mSensorID; };

		timespec getTimeStamp() { return mTimestamp; };

		unsigned long long getCenterFrequency() { return mCenterFrequency; };

		long getSamplingRate() { return mSamplingRate; };

		std::vector<std::complex<float>> &getIQSamples() { return mIQSamples; };
		std::vector<std::complex<float>>& getPSDIQSamples() { return mPSDIQSamples; };
        std::vector<float>& getPSDValues() { return mPSDValues; };

		void setAvroBuffer (char *buffer, unsigned int size) { mAvroBuffer = buffer; mAvroSize=size; };
		char* getAvroBuffer() { return mAvroBuffer; };
		unsigned int getAvroBufferSize() { return mAvroSize; };


	private:

		long long mSensorID;
		struct timespec mTimestamp;
		unsigned long long mCenterFrequency;
		long mSamplingRate;

		std::vector<std::complex<float>> mIQSamples;
		std::vector<std::complex<float>> mPSDIQSamples;
        std::vector<float> mPSDValues;

		char* mAvroBuffer;
		unsigned int mAvroSize;
	};
}
#endif /* SRC_SENSOR_SPECTRUMSEGMENT_H_ */
