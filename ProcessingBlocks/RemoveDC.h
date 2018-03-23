//
// Created by rocapal on 6/10/17.
//

#ifndef ES_SENSOR_REMOVEDC_H
#define ES_SENSOR_REMOVEDC_H


#include <vector>
#include <complex.h>
#include <unistd.h>
#include <algorithm>


#include "../drivers/Component.h"
#include "../types/SpectrumSegment.h"
#include "../context/ElectrosenseContext.h"

namespace electrosense {

    class RemoveDC: public Component<SpectrumSegment*,SpectrumSegment*> {


    public:

        RemoveDC();

        ~RemoveDC(){};

        std::string getNameId () { return std::string("RemoveDC"); };

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
    };

}

#endif //ES_SENSOR_REMOVEDC_H
