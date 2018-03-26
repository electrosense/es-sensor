//
// Created by rocapal on 26/03/18.
//

#ifndef ES_SENSOR_TRANSMISSION_H
#define ES_SENSOR_TRANSMISSION_H

#include <vector>
#include <unistd.h>
#include <string.h>


#include "../drivers/Component.h"
#include "../types/SpectrumSegment.h"
#include "../context/ElectrosenseContext.h"


namespace electrosense {

    class Transmission: public Component<SpectrumSegment*,SpectrumSegment*> {

    public:

        Transmission();

        ~Transmission(){};

        std::string getNameId () { return std::string("Transmission"); };

        int stop();

        ReaderWriterQueue<SpectrumSegment*>* getQueueIn() { return mQueueIn; }
        void setQueueIn (ReaderWriterQueue<SpectrumSegment*>* QueueIn ) { mQueueIn = QueueIn;};

        ReaderWriterQueue<SpectrumSegment*>* getQueueOut() { return NULL; };
        void setQueueOut (ReaderWriterQueue<SpectrumSegment*>* QueueOut) {};



    private:

        void run();
        void parse_tls_hosts();


        bool mRunning;

        ReaderWriterQueue<SpectrumSegment*>* mQueueIn;

        std::string mStrHosts;

        std::string mHost;
        std::string mPort;
        std::string mCACert;
        std::string mCert;
        std::string mKey;


    };

}


#endif //ES_SENSOR_TRANSMISSION_H
