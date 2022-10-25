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

#include "IQSink.h"

namespace openrfsense {

IQSink::IQSink(std::string filename) {

    mFileName = filename;

    mOutputFile.open(filename, std::ios::out | std::ios::binary);
}

float IQSink::remove_DC(float a) { return (a - 127.4) / 128.0; }

void IQSink::run() {

    int samples_c = 0;
    std::cout << "[*] IQSink block running .... " << std::endl;

    mRunning = true;
    SpectrumSegment *segment;

    if (mQueueIn == NULL) {
        throw std::logic_error("Queue[IN] is NULL!");
    }

    unsigned char v[2];
    while (mRunning || mQueueIn->size_approx() != 0) {

        if (mQueueIn && mQueueIn->try_dequeue(segment)) {

            /*
            unsigned char *buf = segment->get_buffer();
            int buf_len = segment->get_buffer_len();


            // Next line save raw data in sequences of IQ 4bytes each one
            std::copy(buf, buf + buf_len, floatArray);

            // Normalize between -1 and 1
            std::vector<float> v_float (floatArray, floatArray +
            sizeof(float)*buf_len);

            std::transform(v_float.begin(), v_float.end(), v_float.begin(),
            std::bind(&IQSink::remove_DC, this, std::placeholders::_1));

             */

            if (OpenRFSenseContext::getInstance()->getOutputType() == OUTPUT_TYPE_BYTE) {
                unsigned char *buf = segment->get_buffer();
                int buf_len = segment->get_buffer_len();

                mOutputFile.write(
                    reinterpret_cast<const char *>(buf), sizeof(unsigned char) * buf_len);

                samples_c = samples_c + buf_len;
            } else if (
                OpenRFSenseContext::getInstance()->getOutputType() ==
                OUTPUT_TYPE_FLOAT) {

                std::vector<float> v_float = segment->getIQ_time_v2();

                int buf_len = segment->get_buffer_len();
                mOutputFile.write(
                    reinterpret_cast<const char *>(v_float.data()),
                    sizeof(float) * v_float.size());

                samples_c = samples_c + v_float.size();
            }

            delete (segment);

        } else
            usleep(0.1);
    }

    mOutputFile.close();

    std::cout << "[*] IQSink: samples written: " << samples_c << std::endl;
}

int IQSink::stop() {

    mRunning = false;
    waitForThread();
    return 1;
}
} // namespace openrfsense
