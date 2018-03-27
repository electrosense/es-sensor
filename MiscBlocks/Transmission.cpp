/*
 * Copyright (C) 2018 by IMDEA Networks Institute
 *
 * This file is part of Electrosense.
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
 * 	Authors: 	Roberto Calvo <roberto.calvo@imdea.org>
 *
 */


#include "Transmission.h"
#include <netinet/in.h>
#include "../misc/TLS.h"


namespace electrosense {

    Transmission::Transmission() {

        mStrHosts = ElectrosenseContext::getInstance()->getTlsHosts();
        parse_tls_hosts();

    }


    void Transmission::run() {

        std::cout << "[*] Transmission block running .... " << std::endl;

        TLS_Connection *tls_con = NULL;
        tls_con = (TLS_Connection *) malloc(sizeof(TLS_Connection *));

        tls_init_p(&tls_con, NULL, TLSv1_1_client_method(),
                 mCACert.c_str(), mCert.c_str(), mKey.c_str(), mHost.c_str(), atoi(mPort.c_str()));

        unsigned int prev_data_size = 0, payload_size, packet_size = 0;
        unsigned int *buf = NULL;

        mRunning = true;
        SpectrumSegment* segment;

        if (mQueueIn==NULL) {
            throw std::logic_error("Queue[IN] is NULL!");
        }


        unsigned int fft_size = 1<<ElectrosenseContext::getInstance()->getLog2FftSize();
        unsigned int reduced_fft_size = (1 - ElectrosenseContext::getInstance()->getFreqOverlap())*(fft_size + 1);

        // We need an odd number of bins
        if (reduced_fft_size % 2 == 0)
            reduced_fft_size++;

        while(mRunning) {

            if (mQueueIn && mQueueIn->try_dequeue(segment)) {

                char* buffer = segment->getAvroBuffer();
                unsigned int data_size = segment->getAvroBufferSize();

                if(data_size != prev_data_size) {
                    // The payload consists of the compressed data plus some padding, guaranteeing the packet size
                    // to be a multiple of 4
                    payload_size = (data_size + 3) & ~0x03;
                    packet_size = 2*sizeof(uint32_t) + payload_size;
                    buf = (uint32_t *) realloc(buf, packet_size);
                    memset(buf, 0, packet_size);
                    prev_data_size = data_size;
                }

                buf[0] = htonl(data_size);
                buf[1] = htonl(reduced_fft_size);

                memcpy(buf+2, buffer, data_size);

                tls_write_p(tls_con, buf, packet_size);

                // Free memory of the segment
                delete(segment);

            }
            else
                usleep(1);
        }

        tls_disconnect(tls_con);
        tls_release(tls_con);

    }


    int Transmission::stop() {

        mRunning = false;
        waitForThread();

        return 1;

    }


    void Transmission::parse_tls_hosts() {

        if(mStrHosts.c_str() != NULL && strcmp(mStrHosts.c_str(), "0") != 0) {

            std::string delim = "#";
            std::string delim_2 = ":";


            auto start = 0U;
            auto end = mStrHosts.find(delim);
            auto iter = 0;
            while (end != std::string::npos)
            {
                if (iter==0) {

                    std::string host = mStrHosts.substr(start, end - start);
                    auto end2 = host.find(delim_2);

                    mHost = host.substr(start, end2);
                    mPort = host.substr(end2+1, end);
                }
                else if (iter == 1) {
                    mCACert = mStrHosts.substr(start, end - start);
                }
                else if (iter == 2) {
                    mCert = mStrHosts.substr(start, end - start);
                    mKey = mStrHosts.substr(end+1, mStrHosts.size()-end);
                }

                iter++;
                start = end + delim.length();
                end = mStrHosts.find(delim, start);
            }

            std::cout << "TLS Server information parsed: " << std::endl;
            std::cout << "   Host:  " << mHost << std::endl;
            std::cout << "   Port:  " << mPort << std::endl;
            std::cout << "   CACer: " << mCACert << std::endl;
            std::cout << "   Cert:  " << mCert << std::endl;
            std::cout << "   Key:   " << mKey << std::endl;



        }
        else
            throw std::logic_error("Transmission - No information about server\n");
    }
}