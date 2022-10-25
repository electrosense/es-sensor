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

#ifndef ORFS_SENSOR_FILESINK_H
#define ORFS_SENSOR_FILESINK_H

#include "../context/OpenRFSenseContext.h"
#include "../drivers/Communication.h"
#include "../drivers/Component.h"
#include "../types/SpectrumSegment.h"

#include <algorithm>
#include <complex.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>
#include <vector>

namespace openrfsense {

class FileSink :
    public Component,
    public Communication<SpectrumSegment *, SpectrumSegment *> {

  public:
    FileSink();

    FileSink(std::string filename);

    ~FileSink(){};

    std::string getNameId() { return std::string("FileSink"); };

    int stop();

    void setFileName(std::string filename) { mFileName = filename; };

    ReaderWriterQueue<SpectrumSegment *> *getQueueIn() { return mQueueIn; }
    void setQueueIn(ReaderWriterQueue<SpectrumSegment *> *QueueIn) {
        mQueueIn = QueueIn;
    };

    ReaderWriterQueue<SpectrumSegment *> *getQueueOut() { return NULL; };
    void setQueueOut(ReaderWriterQueue<SpectrumSegment *> *QueueOut){};

  private:
    void run();

    ReaderWriterQueue<SpectrumSegment *> *mQueueIn;

    std::string mFileName;
    std::ofstream mOutputFile;
};

} // namespace openrfsense

#endif // ORFS_SENSOR_FILESINK_H
