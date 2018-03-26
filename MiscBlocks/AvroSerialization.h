//
// Created by rocapal on 26/03/18.
//

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
