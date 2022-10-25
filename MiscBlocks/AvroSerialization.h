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

#ifndef ORFS_SENSOR_AVROSERIALIZATION_H
#define ORFS_SENSOR_AVROSERIALIZATION_H

#include "../context/OpenRFSenseContext.h"
#include "../drivers/Communication.h"
#include "../drivers/Component.h"
#include "../types/SpectrumSegment.h"

#include <algorithm>
#include <complex.h>
#include <unistd.h>
#include <vector>

extern "C" {
#include <avro.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
};

namespace openrfsense {

class AvroSerialization :
    public Component,
    public Communication<SpectrumSegment *, SpectrumSegment *> {

  public:
    AvroSerialization();

    ~AvroSerialization(){};

    std::string getNameId() { return std::string("AvroSerialization"); };

    int stop();

    void PSD();

    void IQ();

    ReaderWriterQueue<SpectrumSegment *> *getQueueIn() { return mQueueIn; }
    void setQueueIn(ReaderWriterQueue<SpectrumSegment *> *QueueIn) {
      mQueueIn = QueueIn;
    };

    ReaderWriterQueue<SpectrumSegment *> *getQueueOut() { return mQueueOut; };
    void setQueueOut(ReaderWriterQueue<SpectrumSegment *> *QueueOut){};

  private:
    void run();
    void parseSchema(const char *filename, avro_schema_t &avro_schema);
    void writeMetadata(avro_value_t &base, SpectrumSegment *segment);
    int get_mac_address_eth0(char *mac);

    ReaderWriterQueue<SpectrumSegment *> *mQueueOut;
    ReaderWriterQueue<SpectrumSegment *> *mQueueIn;
};

} // namespace openrfsense

#endif // ORFS_SENSOR_AVROSERIALIZATION_H
