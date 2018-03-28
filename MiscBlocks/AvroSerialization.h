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

#ifndef ES_SENSOR_AVROSERIALIZATION_H
#define ES_SENSOR_AVROSERIALIZATION_H


#include <vector>
#include <complex.h>
#include <unistd.h>
#include <algorithm>


#include "../drivers/Component.h"
#include "../types/SpectrumSegment.h"
#include "../context/ElectrosenseContext.h"

extern "C" {
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <avro.h>
};

namespace electrosense {


    class AvroSerialization: public Component<SpectrumSegment*,SpectrumSegment*> {

    public:

        AvroSerialization();

        ~AvroSerialization(){};

        std::string getNameId () { return std::string("AvroSerialization"); };

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

        int get_mac_address_eth0(long long *mac_dec);

    };


}

#endif //ES_SENSOR_AVROSERIALIZATION_H
