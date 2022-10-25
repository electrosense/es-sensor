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

#ifndef ORFS_SENSOR_AVERAGING_H
#define ORFS_SENSOR_AVERAGING_H

#include "../context/OpenRFSenseContext.h"
#include "../drivers/Communication.h"
#include "../drivers/Component.h"
#include "../types/SpectrumSegment.h"

#include <algorithm>
#include <complex.h>
#include <functional>
#include <liquid/liquid.h>
#include <unistd.h>
#include <vector>

namespace openrfsense {

class Averaging :
    public Component,
    public Communication<SpectrumSegment *, SpectrumSegment *> {

  public:
    Averaging();

    ~Averaging(){};

    std::string getNameId() { return std::string("Averaging"); };

    int stop();

    ReaderWriterQueue<SpectrumSegment *> *getQueueIn() { return mQueueIn; }
    void setQueueIn(ReaderWriterQueue<SpectrumSegment *> *QueueIn) {
        mQueueIn = QueueIn;
    };

    ReaderWriterQueue<SpectrumSegment *> *getQueueOut() { return mQueueOut; };
    void setQueueOut(ReaderWriterQueue<SpectrumSegment *> *QueueOut){};

  private:
    void run();

    ReaderWriterQueue<SpectrumSegment *> *mQueueOut;
    ReaderWriterQueue<SpectrumSegment *> *mQueueIn;

    std::vector<SpectrumSegment *> avgVector;
};

} // namespace openrfsense

#endif // ORFS_SENSOR_AVERAGING_H
