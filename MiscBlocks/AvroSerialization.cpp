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


#include "AvroSerialization.h"



namespace electrosense {


    AvroSerialization::AvroSerialization() {
        mQueueOut = new ReaderWriterQueue<SpectrumSegment *>(100);
    }

    void AvroSerialization::run() {

        if (ElectrosenseContext::getInstance()->getPipeline().compare("PSD") == 0 )
            PSD();
        else
            IQ();

    }

#define check_i(call) \
    do { \
        if ((call) != 0) { \
            fprintf(stderr, "Error: %s\n", avro_strerror()); \
            exit(EXIT_FAILURE); \
        } \
} while (0)

    void AvroSerialization::IQ() {

        std::cout << "[*] AvroSerialization IQ block running .... " << std::endl;

        char                 *json_schema;
        const char           *json_schema_file = "schema/iq-spec.avsc";
        avro_schema_t        avro_schema = NULL;


        mRunning = true;
        SpectrumSegment *segment;

        if (mQueueIn == NULL || mQueueOut == NULL) {
            throw std::logic_error("Queue[IN|OUT] are NULL!");
        }


        // Read JSON schema from file
        FILE *file = fopen(json_schema_file, "r");
        if(file == NULL) {
            fprintf(stderr, "[AvroSerialization] Failed to open file %s.\n", json_schema_file);
            exit(1);
        }
        fseek(file, 0, SEEK_END);
        unsigned int file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        json_schema = (char *) malloc(file_size+1);
        size_t read = fread(json_schema, 1, file_size, file);
        if(read != file_size) {
            fprintf(stderr, "[AvroSerialization] File %s incompletely read.\n", json_schema_file);
            exit(1);
        }
        json_schema[file_size] = '\0';
        fclose(file);

        // Parse JSON schema into C API's internal schema representation
        avro_schema_from_json(json_schema, 0, &avro_schema, NULL);
        if(avro_schema == NULL) {
            fprintf(stderr, "[AvroSerialization] Failed to parse AVRO schema.\n");
            exit(1);
        }
        // Release JSON schema
        free(json_schema);

        avro_value_iface_t *avro_iface = avro_generic_class_from_schema(avro_schema);

        long long mac_eth0_dec;
        get_mac_address_eth0(&mac_eth0_dec);

        while(mRunning || mQueueIn->size_approx() != 0) {

            if (mQueueIn && mQueueIn->try_dequeue(segment)) {

                unsigned int s = 4096;

                unsigned int buf_size = ((s+segment->getIQSamples().size()*2*4) + 3) & ~0x03;
                char* buf = (char *) malloc(buf_size);
                if(buf==NULL) {
                    fprintf(stderr, "[AvroSerialization] Error while allocating memory\n");
                    exit(-1);
                }

                // Create Avro memory writer
                avro_writer_t avro_writer = avro_writer_memory(buf, buf_size);

                // Create a value that is an instance of that schema
                avro_value_t avro_value_sample;

                avro_generic_value_new(avro_iface, &avro_value_sample);

                // Sensing ID and timestamps (required)
                avro_value_t avro_value_lossrate, avro_value_sen_id, avro_value_timesecs, avro_value_timemicrosecs,
                        avro_value_sen_env, avro_value_sen_temp, avro_value_sen_latitude,
                        avro_value_sen_longitude, avro_value_sen_altitude, avro_value_sentemp_branch, avro_value_latitude_branch,
                        avro_value_longitude_branch, avro_value_altitude_branch;


                avro_value_get_by_name(&avro_value_sample, "senId", &avro_value_sen_id, NULL);
                avro_value_set_long(&avro_value_sen_id, mac_eth0_dec);

                avro_value_get_by_name(&avro_value_sample, "timeSecs", &avro_value_timesecs, NULL);
                avro_value_set_long(&avro_value_timesecs, segment->getTimeStamp().tv_sec);

                avro_value_get_by_name(&avro_value_sample, "timeMicroSecs", &avro_value_timemicrosecs, NULL);
                avro_value_set_int(&avro_value_timemicrosecs, segment->getTimeStamp().tv_nsec);


                check_i(avro_value_get_by_name(&avro_value_sample, "senEnv", &avro_value_sen_env, NULL));
                check_i(avro_value_get_by_name(&avro_value_sen_env, "senTemp", &avro_value_sen_temp, NULL));
                check_i(avro_value_set_branch(&avro_value_sen_temp, 0, &avro_value_sentemp_branch));
                check_i(avro_value_set_null(&avro_value_sentemp_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_env, "senLatitude", &avro_value_sen_latitude, NULL));
                check_i(avro_value_set_branch(&avro_value_sen_latitude, 0, &avro_value_latitude_branch));
                check_i(avro_value_set_null(&avro_value_latitude_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_env, "senLongitude", &avro_value_sen_longitude, NULL));
                check_i(avro_value_set_branch(&avro_value_sen_longitude, 0, &avro_value_longitude_branch));
                check_i(avro_value_set_null(&avro_value_longitude_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_env, "senAltitude", &avro_value_sen_altitude, NULL));
                check_i(avro_value_set_branch(&avro_value_sen_altitude, 0, &avro_value_altitude_branch));
                check_i(avro_value_set_null(&avro_value_altitude_branch));


                avro_value_t avro_value_sen_conf, avro_value_frontend_gain, avro_value_center_frequency,
                        avro_value_sampling_rate, avro_value_ss_calibrated, avro_value_iq_calibrated,
                        avro_value_noise_floor, avro_value_hopping_strategy, avro_value_antenna_gain,
                        avro_value_freq_corr, avro_value_antenna_id, avro_value_rfsync, avro_value_system_sync,
                        avro_value_extra_conf, avro_value_obfuscation,avro_value_branch;

                check_i(avro_value_get_by_name(&avro_value_sample, "senConf", &avro_value_sen_conf, NULL));


                check_i(avro_value_get_by_name(&avro_value_sen_conf, "hoppingStrategy", &avro_value_hopping_strategy, NULL));
                check_i(avro_value_set_branch(&avro_value_hopping_strategy, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "antennaGain", &avro_value_antenna_gain, NULL));
                check_i(avro_value_set_branch(&avro_value_antenna_gain, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));


                check_i(avro_value_get_by_name(&avro_value_sen_conf, "frontendGain", &avro_value_frontend_gain, NULL));
                check_i(avro_value_set_branch(&avro_value_frontend_gain, 1, &avro_value_branch));
                check_i(avro_value_set_float(&avro_value_branch, ElectrosenseContext::getInstance()->getGain()));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "samplingRate", &avro_value_sampling_rate, NULL));
                check_i(avro_value_set_branch(&avro_value_sampling_rate, 1, &avro_value_branch));
                check_i(avro_value_set_int(&avro_value_branch, ElectrosenseContext::getInstance()->getSamplingRate()));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "centerFreq", &avro_value_center_frequency, NULL));
                check_i(avro_value_set_long(&avro_value_center_frequency, (uint64_t)segment->getCenterFrequency()));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "frequencyCorrectionFactor", &avro_value_freq_corr, NULL));
                check_i(avro_value_set_branch(&avro_value_freq_corr, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "antennaId", &avro_value_antenna_id, NULL));
                check_i(avro_value_set_branch(&avro_value_antenna_id, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "rfSync", &avro_value_rfsync, NULL));
                check_i(avro_value_set_branch(&avro_value_rfsync, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "systemSync", &avro_value_system_sync, NULL));
                check_i(avro_value_set_branch(&avro_value_system_sync, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "sigStrengthCalibration", &avro_value_ss_calibrated, NULL));
                check_i(avro_value_set_branch(&avro_value_ss_calibrated, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "iqBalanceCalibration", &avro_value_iq_calibrated, NULL));
                check_i(avro_value_set_branch(&avro_value_iq_calibrated, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "estNoiseFloor", &avro_value_noise_floor, NULL));
                check_i(avro_value_set_branch(&avro_value_noise_floor, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sen_conf, "extraConf", &avro_value_extra_conf, NULL));
                check_i(avro_value_set_branch(&avro_value_extra_conf, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sample, "lossRate", &avro_value_lossrate, NULL));
                check_i(avro_value_set_branch(&avro_value_lossrate, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));

                check_i(avro_value_get_by_name(&avro_value_sample, "obfuscation", &avro_value_obfuscation, NULL));
                check_i(avro_value_set_branch(&avro_value_obfuscation, 0, &avro_value_branch));
                check_i(avro_value_set_null(&avro_value_branch));




                avro_value_t avro_value_measurements;

                avro_value_get_by_name(&avro_value_sample, "measurements", &avro_value_measurements, NULL);

                avro_value_t avro_value_element;

                std::vector<std::complex<float>> iqsamples = segment->getIQSamples();

                for(unsigned int i=0; i<iqsamples.size(); i++) {
                    size_t new_index;
                    std::complex<float> sample = (segment->getIQSamples()[i]);

                    avro_value_append(&avro_value_measurements, &avro_value_element, &new_index);
                    check_i(avro_value_set_float(&avro_value_element, sample.real()));

                    avro_value_append(&avro_value_measurements, &avro_value_element, &new_index);
                    check_i(avro_value_set_float(&avro_value_element, sample.imag()));

                }


                avro_value_decref(&avro_value_element);

                avro_value_write(avro_writer, &avro_value_sample);

                //std::cout << "Size Writer: " << avro_writer_tell(avro_writer) << std::endl;

                segment->setAvroBuffer(buf, buf_size);

                mQueueOut->enqueue(segment);

                avro_value_decref(&avro_value_sample);
                avro_writer_free(avro_writer);

            } else
                usleep(1);
        }

        avro_value_iface_decref(avro_iface);
        avro_schema_decref(avro_schema);

    }

    void AvroSerialization::PSD() {

        std::cout << "[*] AvroSerialization PSD block running .... " << std::endl;

        char                 *json_schema;
        const char           *json_schema_file = "schema/rtl-spec.avsc";
        avro_schema_t        avro_schema = NULL;

        mRunning = true;
        SpectrumSegment *segment;

        if (mQueueIn == NULL || mQueueOut == NULL) {
            throw std::logic_error("Queue[IN|OUT] are NULL!");
        }

        unsigned int fft_size = 1<<ElectrosenseContext::getInstance()->getLog2FftSize();
        unsigned int reduced_fft_size = (1 - ElectrosenseContext::getInstance()->getFreqOverlap())*(fft_size + 1);
        float freq_res = ((float) ElectrosenseContext::getInstance()->getSamplingRate()) / fft_size;

        // Read JSON schema from file
        FILE *file = fopen(json_schema_file, "r");
        if(file == NULL) {
            fprintf(stderr, "[AvroSerialization] Failed to open file %s.\n", json_schema_file);
            exit(1);
        }
        fseek(file, 0, SEEK_END);
        unsigned int file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        json_schema = (char *) malloc(file_size+1);
        size_t read = fread(json_schema, 1, file_size, file);
        if(read != file_size) {
            fprintf(stderr, "[AvroSerialization] File %s incompletely read.\n", json_schema_file);
            exit(1);
        }
        json_schema[file_size] = '\0';
        fclose(file);

        // Parse JSON schema into C API's internal schema representation
        avro_schema_from_json(json_schema, 0, &avro_schema, NULL);
        if(avro_schema == NULL) {
            fprintf(stderr, "[AvroSerialization] Failed to parse AVRO schema.\n");
            exit(1);
        }
        // Release JSON schema
        free(json_schema);

        avro_value_iface_t *avro_iface = avro_generic_class_from_schema(avro_schema);

        long long mac_eth0_dec;
        get_mac_address_eth0(&mac_eth0_dec);

        while (mRunning) {

            if (mQueueIn && mQueueIn->try_dequeue(segment)) {


                unsigned int buf_size = ((5*4+fft_size*4) + 3) & ~0x03;
                char* buf = (char *) malloc(buf_size);

                // Create Avro memory writer
                avro_writer_t avro_writer = avro_writer_memory(buf, buf_size);

                // Create a value that is an instance of that schema
                avro_value_t avro_value_sample;

                avro_generic_value_new(avro_iface, &avro_value_sample);

                // Unique sensor identifier (required)
                avro_value_t avro_value_sen_id;
                avro_value_get_by_name(&avro_value_sample, "SenId", &avro_value_sen_id, NULL);


                avro_value_set_long(&avro_value_sen_id, mac_eth0_dec);

                // Sensor configuration (required)
                avro_value_t avro_value_sen_conf, avro_value_hopping_strategy, avro_value_windowing_function,
                        avro_value_fft_size, avro_value_averaging_factor, avro_value_frequency_overlap,
                        avro_value_freq_res, avro_value_gain;

                avro_value_get_by_name(&avro_value_sample, "SenConf", &avro_value_sen_conf, NULL);
                avro_value_get_by_name(&avro_value_sen_conf, "HoppingStrategy", &avro_value_hopping_strategy, NULL);
                avro_value_set_int(&avro_value_hopping_strategy, 0); // Sequential
                avro_value_get_by_name(&avro_value_sen_conf, "WindowingFunction", &avro_value_windowing_function, NULL);
                avro_value_set_int(&avro_value_windowing_function, 1); // Hamming
                avro_value_get_by_name(&avro_value_sen_conf, "FFTSize", &avro_value_fft_size, NULL);
                avro_value_set_int(&avro_value_fft_size, fft_size);
                avro_value_get_by_name(&avro_value_sen_conf, "AveragingFactor", &avro_value_averaging_factor, NULL);
                avro_value_set_int(&avro_value_averaging_factor, ElectrosenseContext::getInstance()->getAvgFactor());
                avro_value_get_by_name(&avro_value_sen_conf, "FrequencyOverlap", &avro_value_frequency_overlap, NULL);
                avro_value_set_float(&avro_value_frequency_overlap, ElectrosenseContext::getInstance()->getFreqOverlap());
                avro_value_get_by_name(&avro_value_sen_conf, "FrequencyResolution", &avro_value_freq_res, NULL);
                avro_value_set_float(&avro_value_freq_res, freq_res);
                avro_value_get_by_name(&avro_value_sen_conf, "Gain", &avro_value_gain, NULL);
                avro_value_set_float(&avro_value_gain, ElectrosenseContext::getInstance()->getGain());


                avro_value_t avro_value_sen_pos, avro_value_sen_pos_rec;
                avro_value_get_by_name(&avro_value_sample, "SenPos", &avro_value_sen_pos, NULL);
                //TODO: Sensor position (optional)
                avro_value_set_branch(&avro_value_sen_pos, 0, &avro_value_sen_pos_rec);
                avro_value_set_null(&avro_value_sen_pos_rec);

                // Sensor temperature (optional)
                int set_sen_temp = 0;
                avro_value_t avro_value_sen_temp, avro_value_branch;
                avro_value_get_by_name(&avro_value_sample, "SenTemp", &avro_value_sen_temp, NULL);
                if(set_sen_temp) {
                    avro_value_set_branch(&avro_value_sen_temp, 1, &avro_value_branch);
                    avro_value_set_float(&avro_value_branch, 0.f);
                } else {
                    avro_value_set_branch(&avro_value_sen_temp, 0, &avro_value_branch);
                    avro_value_set_null(&avro_value_branch);
                }

                // Sensor time (required)
                avro_value_t avro_value_sen_time, avro_value_time_secs, avro_value_time_microsecs;
                avro_value_get_by_name(&avro_value_sample, "SenTime", &avro_value_sen_time, NULL);
                avro_value_get_by_name(&avro_value_sen_time, "TimeSecs", &avro_value_time_secs, NULL);
                avro_value_set_long(&avro_value_time_secs, segment->getTimeStamp().tv_sec);
                avro_value_get_by_name(&avro_value_sen_time, "TimeMicrosecs", &avro_value_time_microsecs, NULL);
                avro_value_set_int(&avro_value_time_microsecs, segment->getTimeStamp().tv_nsec);

                // Sensor data (required)
                avro_value_t avro_value_sen_data, avro_value_center_freq, avro_value_squared_mag;
                avro_value_get_by_name(&avro_value_sample, "SenData", &avro_value_sen_data, NULL);
                avro_value_get_by_name(&avro_value_sen_data, "CenterFreq", &avro_value_center_freq, NULL);
                avro_value_set_long(&avro_value_center_freq,  (uint64_t) segment->getCenterFrequency());
                avro_value_get_by_name(&avro_value_sen_data, "SquaredMag", &avro_value_squared_mag, NULL);

                // We need an odd number of bins
                if (reduced_fft_size % 2 == 0)
                    reduced_fft_size++;

                avro_value_t avro_value_element;
                unsigned int l = round((fft_size - reduced_fft_size) / 2);

                for(unsigned int i=0; i<reduced_fft_size; ++i) {
                    size_t new_index;
                    float s = (segment->getPSDValues()[l+i]);

                    avro_value_append(&avro_value_squared_mag, &avro_value_element, &new_index);
                    if(new_index != i) {
                        fprintf(stderr, "[AvroSerialization] ERROR: Unexpected index.\n");
                        exit(1);
                    }
                    avro_value_set_float(&avro_value_element, s);

                }

                avro_value_decref(&avro_value_element);
                avro_value_write(avro_writer, &avro_value_sample);

                segment->setAvroBuffer(buf, buf_size);

                mQueueOut->enqueue(segment);

                avro_value_decref(&avro_value_sample);
                avro_writer_free(avro_writer);

            } else
                usleep(1);
        }

        avro_value_iface_decref(avro_iface);
        avro_schema_decref(avro_schema);

    }

    int AvroSerialization::get_mac_address_eth0(long long *mac_dec) {
        int sock;
        const char *ifname = "eth0";
        char mac_hex[19];
        struct ifreq ifr;

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(sock < 0) return -1;
        strcpy(ifr.ifr_name, ifname);
        ifr.ifr_addr.sa_family = AF_INET;
        if(ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) return -2;

        // MAC address hexadecimal
        sprintf(mac_hex, "%02x%02x%02x%02x%02x%02x",
                (unsigned char) ifr.ifr_hwaddr.sa_data[0],
                (unsigned char) ifr.ifr_hwaddr.sa_data[1],
                (unsigned char) ifr.ifr_hwaddr.sa_data[2],
                (unsigned char) ifr.ifr_hwaddr.sa_data[3],
                (unsigned char) ifr.ifr_hwaddr.sa_data[4],
                (unsigned char) ifr.ifr_hwaddr.sa_data[5]
        );

        // MAC address decimal
        *mac_dec = strtoll(mac_hex, NULL, 16);

        return 0;
    }


    int AvroSerialization::stop() {

        mRunning = false;
        waitForThread();

        return 1;

    }
}