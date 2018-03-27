//
// Created by rocapal on 5/10/17.
//

#ifndef ES_SENSOR_RTLSDR_H
#define ES_SENSOR_RTLSDR_H

/*
 * Copyright (C) 2017 by Electrosense
 *
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
 * 	    Roberto Calvo-Palomino <roberto.calvo@imdea.org>
 *
 */


#include <iostream>
#include <string>
#include <rtl-sdr.h>
#include <thread>
#include <zconf.h>
#include <complex.h>

#include "../Driver.h"
#include "../Component.h"
#include "rtlsdrDriver.h"
#include "../../context/ElectrosenseContext.h"
#include "../common/SequentialHopping.h"
#include "../../types/SpectrumSegment.h"

extern "C" {
#include "converter/converter.h"
};

namespace electrosense {



    class rtlsdrDriver: public Driver, public Component<int, SpectrumSegment*> {

    public:

        rtlsdrDriver();

        ~rtlsdrDriver();

        // Open the device
        int open(std::string deviceId);

        // Close the device
        int close();

        // Stop
        int stop();

        ReaderWriterQueue<int>* getQueueIn() { return NULL; }
        void setQueueIn (ReaderWriterQueue<int>* QueueIn ) {};

        ReaderWriterQueue<SpectrumSegment*>* getQueueOut() { return mQueueOut; };
        void setQueueOut (ReaderWriterQueue<SpectrumSegment*>* QueueOut) {};

        std::string getNameId () { return std::string("rtlsdrDriver"); };


    private:

        const unsigned int MAX_FREQ_RTL_SDR = 1766000000;
        const std::string CONVERTER_PATH = "/dev/esenseconv";

        // Run the driver in the thread
        void run();

        bool mRunning;

        int mDeviceId;
        rtlsdr_dev_t* mDevice;

        SequentialHopping* mSeqHopping;

        ReaderWriterQueue<SpectrumSegment*>* mQueueOut;


        converter mConverterDriver;
        bool mConverterEnabled;

    };

}

#endif //ES_SENSOR_RTLSDR_H
