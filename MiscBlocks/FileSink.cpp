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


#include "FileSink.h"

namespace electrosense {

    FileSink::FileSink(std::string filename) {

        mFileName = filename;
        mOutputFile.open(filename);
    }


    void FileSink::run() {

        std::cout << "[*] FileSink block running .... " << std::endl;

        mRunning = true;
        SpectrumSegment* segment;

        if (mQueueIn==NULL) {
            throw std::logic_error("Queue[IN] is NULL!");
        }

        while(mRunning) {

            if (mQueueIn && mQueueIn->try_dequeue(segment)) {

                mOutputFile << segment->getTimeStamp().tv_sec << "." <<  segment->getTimeStamp().tv_nsec << "," ;
                mOutputFile << segment->getCenterFrequency() << ",";
                std::vector<float> psdsamples = segment->getPSDValues();

                for (unsigned int i=0; i<psdsamples.size(); i++)
                    mOutputFile << std::setprecision(5) << psdsamples.at(i) << "," ;

                mOutputFile << std::endl;

                mOutputFile.flush();

            }
            else
                usleep(1);
        }

        mOutputFile.close();
    }


    int FileSink::stop() {

        mRunning = false;
        waitForThread();

        return 1;

    }
}
