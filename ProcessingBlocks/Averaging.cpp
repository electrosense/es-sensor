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

#include "Averaging.h"

#include <sys/param.h>

namespace openrfsense {

Averaging::Averaging() { mQueueOut = new ReaderWriterQueue<SpectrumSegment *>(100); }

void Averaging::run() {

    std::cout << "[*] Averaging block running .... " << std::endl;

    unsigned int fft_size = 1 << OpenRFSenseContext::getInstance()->getLog2FftSize();
    unsigned int avg_factor = OpenRFSenseContext::getInstance()->getAvgFactor();

    mRunning = true;
    SpectrumSegment *segment;

    if (mQueueIn == NULL || mQueueOut == NULL) {
        throw std::logic_error("Queue[IN|OUT] are NULL!");
    }

    while (mRunning) {

        if (mQueueIn && mQueueIn->try_dequeue(segment)) {

            avgVector.push_back(segment);

            if (avgVector.size() == avg_factor) { // avg_factor) {

                std::vector<float> psd_avg;
                bool wrong_cf = false;
                psd_avg.reserve(fft_size);
                psd_avg.clear();

                // Double Check that all segments refer to the same center
                // frequency
                for (unsigned int i = 1; i < avg_factor; i++)
                    if (avgVector[i - 1]->getCenterFrequency() !=
                        avgVector[i]->getCenterFrequency())
                        wrong_cf = true;

                if (wrong_cf) {
                    std::cerr << "[WARNING] AVG, different center frequency in "
                                 "the segments!"
                              << std::endl;
                    continue;
                }

                for (unsigned int i = 0; i < fft_size; i++) {

                    float power = 0.0;

                    for (unsigned int j = 0; j < avgVector.size(); j++) {

                        power = power +
                                std::pow(avgVector[j]->getIQSamplesFreq()[i].real(), 2) +
                                std::pow(avgVector[j]->getIQSamplesFreq()[i].imag(), 2);
                    }

                    psd_avg.push_back(MAX(10.0f * log10(power / avg_factor), -100));
                }

                // We use the first segment to save the averaged one and we send
                // it through the queue.
                avgVector[0]->getPSDValues() = psd_avg;
                mQueueOut->enqueue(avgVector[0]);

                // Release memory (except 1st element (index=0))
                for (unsigned int i = 1; i < avgVector.size(); i++)
                    delete (avgVector[i]);

                avgVector.clear();
                psd_avg.clear();
            }

        } else
            usleep(1);
    }
}

int Averaging::stop() {

    mRunning = false;
    waitForThread();

    return 1;
}
} // namespace openrfsense
