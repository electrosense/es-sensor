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

#ifndef ES_SENSOR_WINDOWING_H
#define ES_SENSOR_WINDOWING_H

#include <vector>
#include <complex.h>
#include <unistd.h>
#include <algorithm>
#include <liquid/liquid.h>


#include "../drivers/Component.h"
#include "../types/SpectrumSegment.h"
#include "../context/ElectrosenseContext.h"

namespace electrosense {

    class Windowing: public Component<SpectrumSegment*,SpectrumSegment*> {

    public:

        enum W_FUNCTION {
            HAMMING=0,
            BLACKMAN_HARRIS};

        Windowing(W_FUNCTION window_func);

        ~Windowing(){};

        std::string getNameId () { return std::string("Windowing"); };

        int stop();

        ReaderWriterQueue<SpectrumSegment*>* getQueueIn() { return mQueueIn; }
        void setQueueIn (ReaderWriterQueue<SpectrumSegment*>* QueueIn ) { mQueueIn = QueueIn;};

        ReaderWriterQueue<SpectrumSegment*>* getQueueOut() { return mQueueOut; };
        void setQueueOut (ReaderWriterQueue<SpectrumSegment*>* QueueOut) {};

    private:

        void run();

        bool mRunning;

        ReaderWriterQueue<SpectrumSegment*>* mQueueOut;
        ReaderWriterQueue<SpectrumSegment*>* mQueueIn;

        std::vector<float> w_func;
    };
}


#endif //ES_SENSOR_WINDOWING_H
