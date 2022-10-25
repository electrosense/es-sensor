
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
 * 	            Roberto
 *
 */
#ifndef OPENRFSENSE_SENSOR_IQSTREAM_H
#define OPENRFSENSE_SENSOR_IQSTREAM_H

#include <algorithm>
#include <complex.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <vector>

//#include "mysocket.h"

#include "../context/OpenRFSenseContext.h"
#include "../drivers/Communication.h"
#include "../drivers/Component.h"
#include "../types/SpectrumSegment.h"
#include "Filter/CIC/CIC.h"
#include "Filter/FIR/FIRDesign.h"
#include "Filter/FIR/RationalFIRFilter.h"
#include "Filter/Resampler/Downsampler.h"

#include <endian.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

namespace openrfsense {

class IQStream :
    public Component,
    public Communication<SpectrumSegment *, SpectrumSegment *> {

  public:
    IQStream();

    ~IQStream(){};

    std::string getNameId() { return std::string("IQStream"); };

    int stop();

    void setFileName(std::string filename) { mFileName = filename; };

    ReaderWriterQueue<SpectrumSegment *> *getQueueIn() { return mQueueIn; }
    void setQueueIn(ReaderWriterQueue<SpectrumSegment *> *QueueIn) {
        mQueueIn = QueueIn;
    };

    ReaderWriterQueue<SpectrumSegment *> *getQueueOut() { return NULL; };
    void setQueueOut(ReaderWriterQueue<SpectrumSegment *> *QueueOut){};

    bool getDecoderState() {
        mMutex.lock();
        bool state = mDecoderState;
        mMutex.unlock();
        return state;
    };

  private:
    void initSocket();
    ssize_t sendIQSamples(
        int fd, uint64_t centerFrequency, uint32_t gain, uint32_t nSamples,
        const int8_t *iq);

    void run();

    void setDecoderState(bool state) {
        mMutex.lock();
        mDecoderState = state;
        mMutex.unlock();
    };

    std::mutex mMutex;
    bool mDecoderState;

    ReaderWriterQueue<SpectrumSegment *> *mQueueIn;

    std::string mFileName;

    struct Header {
        uint64_t centerFrequency;
        uint32_t gain;
        uint32_t nSamples;
    } __attribute__((packed));

    int m_cfd;
    int m_fd;
    bool mSocketEnable;
    struct sockaddr_un addr;

    int mChildPID;
};

} // namespace openrfsense

#endif // OPENRFSENSE_SENSOR_IQSINK_H
