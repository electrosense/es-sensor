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

#include "PSDFast.h"

#include <sys/param.h>

//#define SERVER "192.168.0.118"
//#define SERVER "192.168.0.113"

#define SERVER "127.0.0.1"
#define PORT   5555

namespace openrfsense {

PSDFast::PSDFast() {

    mQueueOut = new ReaderWriterQueue<SpectrumSegment *>(100);

    if ((mSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        error(EXIT_FAILURE, errno, "accept failed");
    }

    memset(&si_other, 0, sizeof(si_other));

    si_other.sin_family = AF_INET;
    si_other.sin_addr.s_addr = inet_addr(SERVER);
    si_other.sin_port = htons(PORT);
}

void PSDFast::run() {

    std::cout << "[*] PSDFast block running .... " << std::endl;

    uint32_t signal_len = 1024; // samples.

    mRunning = true;
    SpectrumSegment *segment;

    if (mQueueIn == NULL) { // || mQueueOut==NULL) {
        throw std::logic_error("Queue[IN|OUT] are NULL!");
    }

    int flags = 0;

    std::vector<float> w_func;
    // Windowing
    float W = 0;
    for (int i = 0; i < signal_len / 2; i++) {
        auto w_i = hamming(i + 1, signal_len / 2);
        w_func.push_back(w_i);
        W += std::pow(w_i, 2);
    }

    while (mRunning) {

        if (mQueueIn && mQueueIn->try_dequeue(segment)) {

            unsigned char *buf = segment->get_buffer();

            std::complex<float> signal[signal_len / 2];
            std::complex<float> signal_freq[signal_len / 2];

            // Prepare the data
            for (uint32_t t = 0; t < signal_len; t = t + 2) {
                signal[t / 2] = std::complex<float>(
                    ((buf[t] - 127.4) / 128.0), ((buf[t + 1] - 127.4) / 128.0));
                signal[t / 2] = signal[t / 2] * std::sqrt((float)(signal_len / 2.0) / W);
            }

            // FFT
            fftplan q_f = fft_create_plan(
                signal_len / 2, signal, signal_freq, LIQUID_FFT_FORWARD, flags);
            fft_execute(q_f);
            fft_shift(signal_freq, signal_len / 2);

            fft_destroy_plan(q_f);

            // Power computation
            signed char power[signal_len / 2];

            for (uint32_t t = 0; t < signal_len / 2; t = t + 1) {

                double p = std::pow(signal_freq[t].real(), 2) +
                           std::pow(signal_freq[t].imag(), 2);
                power[t] = static_cast<signed char>(round(MAX(10.0f * log10(p), -100)));

                /*
                                    if (t+1 == signal_len/2)
                                        printf("%d \n ", power[t]);
                                    else
                                        printf("%d , ", power[t]);
                */
            }

            unsigned int buffer_len = sizeof(unsigned char) + sizeof(double) +
                                      sizeof(double) + sizeof(uint32_t) +
                                      sizeof(power); // 512 535
            unsigned char buffer[buffer_len];
            // char buffer[buffer_len];
            memset(&buffer, 0, sizeof(buffer));

            uint64_t center_freq = OpenRFSenseContext::getInstance()->getMinFreq();

            unsigned char id = 1;
            buffer[0] = id; // Id

            double minFreq = (center_freq - 1200000);
            memcpy(buffer + sizeof(id), &minFreq, sizeof(minFreq));

            double maxFreq = (center_freq + 1200000);
            memcpy(buffer + sizeof(id) + sizeof(minFreq), &maxFreq, sizeof(maxFreq));

            uint32_t data_len = signal_len / 2;
            memcpy(
                buffer + sizeof(id) + sizeof(minFreq) + sizeof(maxFreq), &data_len,
                sizeof(data_len));

            // Power data
            memcpy(
                buffer + sizeof(id) + sizeof(minFreq) + sizeof(maxFreq) +
                    sizeof(data_len),
                &power, sizeof(power));

            unsigned int n = sendto(
                mSocket, (const int *)buffer, buffer_len, MSG_CONFIRM,
                (struct sockaddr *)&si_other, sizeof(si_other));

            // Control channel read
            // Read JSON file

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 5000;
            if (setsockopt(mSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
                perror("Error");
            }

            unsigned char buffer2[65536];
            unsigned int socklen;
            struct sockaddr_in saddr;
            int status =
                recvfrom(mSocket, buffer2, 65536, 0, (struct sockaddr *)&saddr, &socklen);

            /**CHANGE IT!!! ***/
            // continue;

            // Read JSON from the data channel connected to WebRTC. It uses same
            // UDP port that the PSD data
            //
            if (status > 0) {

                buffer2[status] = '\0';
                printf("Received: '%s'\n", buffer2);
                // std::string
                // str("'{\"decoder\":1,\"decoder_settings\":null,\"fc\":102535391,\"fs\":2400000,\"gain\":0,\"target\":\"es_sensor\"}'");
                // decoder_settings and gain

                std::string str(buffer2, buffer2 + status);

                struct json_object *jobj;
                jobj = json_tokener_parse(str.c_str());

                struct json_object *json_fc;
                json_object_object_get_ex(jobj, "fc", &json_fc);

                uint64_t freq = json_object_get_int64(json_fc);
                std::cout << "frequency: " << freq << std::endl;
                OpenRFSenseContext::getInstance()->setMinFreq(freq);

                struct json_object *json_decoder;
                json_object_object_get_ex(jobj, "decoder", &json_decoder);
                int32_t decoder = json_object_get_int(json_decoder);

                OpenRFSenseContext::getInstance()->setDecoder(decoder);
                std::cout << "Decoder: " << decoder << std::endl;

                struct json_object *json_gain_rtl;
                json_object_object_get_ex(jobj, "gain", &json_gain_rtl);
                int32_t gain_rtl = json_object_get_int(json_gain_rtl);
                float g_rtl = static_cast<float>(gain_rtl);

                OpenRFSenseContext::getInstance()->setGain(g_rtl);

                std::cout << "Gain(rtl-sdr): " << g_rtl << std::endl;

                struct json_object *json_settings, *json_gain;

                json_object_object_get_ex(jobj, "decoder_settings", &json_settings);

                if (json_settings != NULL) {
                    json_object_object_get_ex(json_settings, "gain", &json_gain);
                    int32_t gain = json_object_get_int(json_gain);
                    OpenRFSenseContext::getInstance()->setGainDecoder(gain);

                    std::cout << "Gain (decoder): " << gain << std::endl;

                    free(json_gain);
                    free(json_settings);
                }

                free(json_gain_rtl);
                free(json_decoder);
                free(json_fc);
                free(jobj);
            }

            delete (segment);

        } else
            usleep(0.1);
    }
}

int PSDFast::stop() {

    close(mSocket);

    mRunning = false;
    waitForThread();

    return 1;
}
} // namespace openrfsense
