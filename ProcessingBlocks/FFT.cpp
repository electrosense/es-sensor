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

#include "FFT.h"


namespace electrosense {


    FFT::FFT() {
        mQueueOut = new ReaderWriterQueue<SpectrumSegment *>(100);
    }

    void FFT::run() {

        std::cout << "[*] FFT block running .... " << std::endl;

        unsigned int fft_batch_len = ElectrosenseContext::getInstance()->getFFTbatchlen();

        mRunning = true;
        SpectrumSegment* segment;

        if (mQueueIn==NULL || mQueueOut==NULL) {
            throw std::logic_error("Queue[IN|OUT] are NULL!");
        }

        while(mRunning) {

            if (mQueueIn && mQueueIn->try_dequeue(segment)) {

                mFFTBatch.push_back(segment);

                if (mFFTBatch.size() == fft_batch_len) {//fft_batch_len {

                    // Wait for FFT_batch segments to prepare data and send to the CPU
                    ComputeFFT(mFFTBatch);

//                    std::cout << mFFTBatch[3]->getPSDIQSamples().at(10).real() << " , " <<  mFFTBatch[3]->getPSDIQSamples().at(10).imag() << std::endl ;

/*
                    std::cout << "FFT ****************" << std::endl;
                    for (unsigned int i=0; i<mFFTBatch[0]->getPSDIQSamples().size(); i++) {

                        std::complex<float> aux =  mFFTBatch[0]->getPSDIQSamples().at(i);
                        std::cout << aux.real() << "," << aux.imag() << ",";
                    }
                    std::cout << "FFT ****************" << std::endl;
*/

                    for (unsigned int i=0; i<mFFTBatch.size(); i++)
                        mQueueOut->enqueue(mFFTBatch[i]);


                    mFFTBatch.clear();
                }

            }
            else
                usleep(1);
        }
    }

    void FFT::ComputeFFT (std::vector<SpectrumSegment*>& segments) {

        unsigned int signal_len = 1<<ElectrosenseContext::getInstance()->getLog2FftSize();
        int flags=0;
        std::complex<float> signal_freq[signal_len];


        for (int i=0; i<segments.size(); i++) {

            std::complex<float>* signal = segments[i]->getIQSamples().data();

            //std::cout << segments[i]->getIQSamples()[index].real() << " , " << segments[i]->getIQSamples()[index].imag() << std::endl;
            //std::cout << signal[index].real() << " , " << signal[index].imag() << std::endl;


            fftplan q_f = fft_create_plan(signal_len, signal, signal_freq, LIQUID_FFT_FORWARD, flags);
            fft_execute(q_f);
            fft_shift(signal_freq,signal_len);


            segments[i]->getPSDIQSamples().assign(signal_freq, signal_freq+signal_len);

            fft_destroy_plan(q_f);

            //std::cout << signal_freq[index].real() << " , " << signal_freq[index].imag() << std::endl;
            //std::cout << segments[i]->getPSDSamples()->at(index).real() << " , " << segments[i]->getPSDIQSamples()->at(index).imag() << std::endl;

        }

    //    std::cout << segments[3]->getPSDIQSamples().at(10).real() << " , " <<  segments[3]->getPSDIQSamples().at(10).imag() << std::endl ; //<< " | " << signal_freq[10].real() << " , " << signal_freq[10].imag() << std::endl;

    }



    int FFT::stop() {

        mRunning = false;
        waitForThread();

        return 1;

    }
}