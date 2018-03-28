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

#include "Windowing.h"

namespace electrosense {


    Windowing::Windowing(W_FUNCTION window_func) {
        mQueueOut = new ReaderWriterQueue<SpectrumSegment *>(100);

    }

    void Windowing::run() {

        std::cout << "[*] Windowing block running .... " << std::endl;

        // Build the windowing function
        int fft_size = 1<<ElectrosenseContext::getInstance()->getLog2FftSize();

        for (int i=0; i<fft_size; i++)
        {
            w_func.push_back( hamming(i+1,fft_size));
        }

        mRunning = true;
        SpectrumSegment* segment;

        if (mQueueIn==NULL || mQueueOut==NULL) {
            throw std::logic_error("Queue[IN|OUT] are NULL!");
        }

        while(mRunning) {

            if (mQueueIn && mQueueIn->try_dequeue(segment)) {

                // Perform windowing faction over the segments
                std::vector<std::complex<float>>& iq_complex = segment->getIQSamples();

                for (unsigned int p = 0; p < iq_complex.size(); p = p + 1) {
                    std::complex<float> aux = iq_complex[p];
                    iq_complex[p] = std::complex<float> ( (aux.real() * w_func[p]), (aux.imag() *w_func[p]) );
                }

                mQueueOut->enqueue(segment);
            }
            else
                usleep(1);
        }
    }


    int Windowing::stop() {

        mRunning = false;
        waitForThread();

        return 1;

    }
}

