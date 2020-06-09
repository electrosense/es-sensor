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
 * 	Authors:
 * 	    Roberto Calvo-Palomino <roberto.calvo@imdea.org>
 *
 */
#ifndef ES_SENSOR_RTLSDR_H
#define ES_SENSOR_RTLSDR_H

#include <iostream>
#include <rtl-sdr.h>
#include <string>
#include <thread>
#include <zconf.h>

#include <endian.h>
#include <errno.h>
#include <error.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "../../context/ElectrosenseContext.h"
#include "../Communication.h"
#include "../Component.h"
#include "../Driver.h"
#include "../common/SequentialHopping.h"
#include "rtlsdrDriver.h"

// Workaround issue #4 , complex.h breaks openssl's RSA library
//   include RSA before any mention to complex.h
#include "../../types/SpectrumSegment.h"
#include <complex.h>

extern "C" {
#include "converter/converter.h"
};

namespace electrosense {

class rtlsdrDriver : public Driver,
                     public Component,
                     public Communication<int, SpectrumSegment *> {

public:
  rtlsdrDriver();

  ~rtlsdrDriver();

  // Open the device
  int open(std::string deviceId);

  // Close the device
  int close();

  // Stop
  int stop();

  void SyncSampling();

  void AsyncSampling();

  ReaderWriterQueue<int> *getQueueIn() { return NULL; }
  void setQueueIn(ReaderWriterQueue<int> *QueueIn){};

  ReaderWriterQueue<SpectrumSegment *> *getQueueOut() { return mQueueOut; };
  ReaderWriterQueue<SpectrumSegment *> *getQueueOut2() { return mQueueOut2; };
  void setQueueOut(ReaderWriterQueue<SpectrumSegment *> *QueueOut){};

  std::string getNameId() { return std::string("rtlsdrDriver"); };

private:
  const std::string CONVERTER_PATH = "/dev/esenseconv";

  // Run the driver in the thread
  void run();

  int mDeviceId;
  rtlsdr_dev_t *mDevice;

  SequentialHopping *mSeqHopping;

  ReaderWriterQueue<SpectrumSegment *> *mQueueOut;
  ReaderWriterQueue<SpectrumSegment *> *mQueueOut2;

  converter mConverterDriver;
  bool mConverterEnabled;

  // static void* socket_thread( void *arg);

  std::vector<std::complex<float>> m_capbuf_raw;
};

} // namespace electrosense

#endif // ES_SENSOR_RTLSDR_H
