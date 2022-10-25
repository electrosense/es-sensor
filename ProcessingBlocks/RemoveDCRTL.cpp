/*
 * Copyright (C) 2019 by IMDEA Networks Institute
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

#include "RemoveDCRTL.h"

namespace openrfsense {

RemoveDCRTL::RemoveDCRTL() { mQueueOut = new ReaderWriterQueue<SpectrumSegment *>(100); }

float RemoveDCRTL::remove_DC(float a) { return (a - 127.4) / 128.0; }
void RemoveDCRTL::run() {

    std::cout << "[*] RemoveDCRLT block running .... " << std::endl;

    mRunning = true;
    SpectrumSegment *segment;

    if (mQueueIn == NULL || mQueueOut == NULL) {
        throw std::logic_error("Queue[IN|OUT] are NULL!");
    }

    float *floatArray = (float *)malloc(sizeof(float) * MAX_SEGMENT);

    while (mRunning || mQueueIn->size_approx() != 0) {

        if (mQueueIn && mQueueIn->try_dequeue(segment)) {

            unsigned char *buf = segment->get_buffer();
            int buf_len = segment->get_buffer_len();

            std::copy(buf, buf + buf_len, floatArray);
            std::vector<float> v_float(floatArray, floatArray + sizeof(float) * buf_len);
            v_float.resize(buf_len);

            // Remove DC and normalize between -1 and 1
            std::transform(
                v_float.begin(), v_float.end(), v_float.begin(),
                std::bind(&RemoveDCRTL::remove_DC, this, std::placeholders::_1));

            // std::copy(v_float.begin(), v_float.end(),
            // segment->getIQ_time_v2().begin());
            segment->getIQ_time_v2().assign(v_float.begin(), v_float.end());

            mQueueOut->enqueue(segment);

        } else
            usleep(1);
    }

    free(floatArray);
}

int RemoveDCRTL::stop() {

    mRunning = false;
    waitForThread();

    return 1;
}

} // namespace openrfsense