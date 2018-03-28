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

#include "RemoveDC.h"


namespace electrosense {


RemoveDC::RemoveDC() {
    mQueueOut = new ReaderWriterQueue<SpectrumSegment*>(100);
}


void RemoveDC::run() {

    std::cout << "[*] RemoveDC block running .... " << std::endl;

    mRunning = true;
    SpectrumSegment* segment;

    if (mQueueIn==NULL || mQueueOut==NULL) {
        throw std::logic_error("Queue[IN|OUT] are NULL!");
    }


    while(mRunning) {


        if (mQueueIn && mQueueIn->try_dequeue(segment)) {

            std::vector<std::complex<float>>& iq_complex = segment->getIQSamples();

            /*
            double real_acc=0, imag_acc=0;

            for (auto c: iq_complex) {
                real_acc += real(c);
                imag_acc += imag(c);
            }

            double mean_real = real_acc / iq_complex.size();
            double mean_imag = imag_acc / iq_complex.size();


            //std::transform(iq_complex.begin(), iq_complex.end(), iq_complex.begin(),
            //               [](std::complex<double> c) -> std::complex<double> { std::complex<double>(real(c)-mean_real,imag(c)-mean_imag); } );

            // Check the performance between std::transform and std::for_each

            std::for_each(iq_complex.begin(), iq_complex.end(), [mean_real, mean_imag](std::complex<double> c) {
                c.real(c.real() - mean_real);
                c.imag(c.imag() - mean_imag);
            } );
            */
/*
            std::cout << "RemoveDC ****************" << std::endl;
            for (unsigned int p = 0; p < iq_complex.size(); p = p + 1) {
                std::complex<float> aux = iq_complex[p];
                std::cout << aux.real() << "," << aux.imag() << ",";
            }
            std::cout << "RemoveDC ****************" << std::endl;
*/
            for (unsigned int p = 0; p < iq_complex.size(); p = p + 1) {
                std::complex<float> aux = iq_complex[p];
                iq_complex[p] = std::complex<float> ( (aux.real() - 127.4)/128.0, (aux.imag() - 127.4)/128.0);
            }

/*
            std::cout << "RemoveDC ****************" << std::endl;
            for (unsigned int p = 0; p < iq_complex.size(); p = p + 1) {
                std::complex<float> aux = iq_complex[p];
                std::cout << aux.real() << "," << aux.imag() << ",";
            }
            std::cout << "RemoveDC ****************" << std::endl;
*/

            mQueueOut->enqueue(segment);

        } else
            usleep(1);

    }

}

int RemoveDC::stop() {

    mRunning = false;
    waitForThread();

    return 1;
}



}