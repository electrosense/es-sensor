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


namespace electrosense {

    IQSink::IQSink(std::string filename) {

        mFileName = filename;
        mOutputFile.open(filename, std::ios::out | std::ios::binary);
    }


    void IQSink::run() {

        int samples_c = 0;
        std::cout << "[*] IQSink block running .... " << std::endl;

        mRunning = true;
        SpectrumSegment* segment;

        if (mQueueIn==NULL) {
            throw std::logic_error("Queue[IN] is NULL!");
        }

        while(mRunning) {

            if (mQueueIn && mQueueIn->try_dequeue(segment)) {

                //std::cout << "[o] Receiving segment " << segment->getTimeStamp().tv_sec << "." << segment->getTimeStamp().tv_nsec << std::endl;

                std::vector<std::complex<float>> iq = segment->getIQSamples();

                for (unsigned int i=0; i<iq.size(); i++) {
                      float re = (iq[i].real() -127.0)/128.0;
                      float im = (iq[i].imag() -127.0)/128.0;


                    mOutputFile.write(reinterpret_cast<const char *>( &re ), sizeof(float));
                    mOutputFile.write(reinterpret_cast<const char *>( &im ), sizeof(float));

                    samples_c = samples_c + 2;

                }


                mOutputFile.flush();

                delete(segment);

            }
            else
                usleep(1);
        }

        mOutputFile.close();

        //std::cout << "[*] IQSink: samples written: " << samples_c << std::endl;

    }


    int IQSink::stop() {

        mRunning = false;
        waitForThread();

        return 1;

    }
}