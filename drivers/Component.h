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

#ifndef ES_SENSOR_COMPONENT_H
#define ES_SENSOR_COMPONENT_H

#include <thread>

#include "../misc/readerwriterqueue.h"

using namespace moodycamel;

template <typename Tin, typename Tout>
class Component {

public:

    typedef const Tin ParamIn;
    typedef const Tout ParamOut;

    // Getters for the queue
    virtual ReaderWriterQueue<Tin>* getQueueIn() = 0;
    virtual ReaderWriterQueue<Tout>* getQueueOut() = 0;

    // Setters for the queue
    virtual void setQueueIn (ReaderWriterQueue<Tin>* QueueIn) = 0;
    virtual void setQueueOut (ReaderWriterQueue<Tout>* QueueOut) = 0;

    // Name identifier for the component
    virtual std::string getNameId () = 0;

    // Generic function to start the component in a separated thread.
    void start () {mThread = std::thread ( [this] { run(); } );};

    // This function should implement how the thread must finish
    virtual int stop () = 0;

    // wait
    void waitForThread () { mThread.join(); };

private:

    // method that executes in the thread
    virtual void run () = 0;


    // thread
    std::thread mThread;

};

#endif