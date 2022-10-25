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

#include "FFT.h"

namespace openrfsense {

FFT::FFT() { mQueueOut = new ReaderWriterQueue<SpectrumSegment *>(100); }

void FFT::run() {

    std::cout << "[*] FFT block running .... " << std::endl;

    unsigned int fft_batch_len = OpenRFSenseContext::getInstance()->getFFTbatchlen();

    mRunning = true;
    SpectrumSegment *segment;

    if (mQueueIn == NULL || mQueueOut == NULL) {
        throw std::logic_error("Queue[IN|OUT] are NULL!");
    }

    while (mRunning) {

        if (mQueueIn && mQueueIn->try_dequeue(segment)) {

            mFFTBatch.push_back(segment);

            if (mFFTBatch.size() == fft_batch_len) { // fft_batch_len {

                // Wait for FFT_batch segments to prepare data and send to the
                // CPU
                ComputeFFT(mFFTBatch);

                for (unsigned int i = 0; i < mFFTBatch.size(); i++)
                    mQueueOut->enqueue(mFFTBatch[i]);

                mFFTBatch.clear();
            }

        } else
            usleep(1);
    }
}

void FFT::ComputeFFT(std::vector<SpectrumSegment *> &segments) {

    unsigned int signal_len = 1 << OpenRFSenseContext::getInstance()->getLog2FftSize();
    int flags = 0;
    std::complex<float> signal_freq[signal_len];

    for (unsigned int i = 0; i < segments.size(); i++) {

        std::complex<float> *signal = segments[i]->getIQSamples().data();

        fftplan q_f =
            fft_create_plan(signal_len, signal, signal_freq, LIQUID_FFT_FORWARD, flags);
        fft_execute(q_f);
        fft_shift(signal_freq, signal_len);

        segments[i]->getIQSamplesFreq().assign(signal_freq, signal_freq + signal_len);

        fft_destroy_plan(q_f);
    }
}

int FFT::stop() {

    mRunning = false;
    waitForThread();

    return 1;
}
} // namespace openrfsense