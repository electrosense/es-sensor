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

namespace openrfsense {

AvroSerialization::AvroSerialization() {
    mQueueOut = new ReaderWriterQueue<SpectrumSegment *>(100);
}

void AvroSerialization::run() {
    if (OpenRFSenseContext::getInstance()->getPipeline().compare("PSD") == 0)
        PSD();
    else
        IQ();
}

#define check_i(call)                                        \
    do {                                                     \
        if ((call) != 0) {                                   \
            fprintf(stderr, "Error: %s\n", avro_strerror()); \
            exit(EXIT_FAILURE);                              \
        }                                                    \
    } while (0)

void AvroSerialization::IQ() {
    std::cout << "[*] AvroSerialization IQ block running .... " << std::endl;

    mRunning = true;
    SpectrumSegment *segment;

    if (mQueueIn == NULL || mQueueOut == NULL) {
        throw std::logic_error("Queue[IN|OUT] are NULL!");
    }

    avro_schema_t avro_schema = NULL;
    parseSchema("schemas/iq-spec.avsc", avro_schema);

    avro_value_iface_t *avro_iface = avro_generic_class_from_schema(avro_schema);

    while (mRunning || mQueueIn->size_approx() != 0) {
        if (mQueueIn && mQueueIn->try_dequeue(segment)) {
            unsigned int s = 4096;

            unsigned int buf_size =
                ((s + segment->getIQ_time_v2().size() * 2 * 4) + 3) & ~0x03;
            char *buf = (char *)malloc(buf_size);
            if (buf == NULL) {
                fprintf(stderr, "[AvroSerialization] Error while allocating memory\n");
                exit(-1);
            }

            // Create Avro memory writer
            avro_writer_t avro_writer = avro_writer_memory(buf, buf_size);

            // Create a value that is an instance of that schema
            avro_value_t avro_value_sample;
            avro_generic_value_new(avro_iface, &avro_value_sample);

            // Populate metadata
            writeMetadata(avro_value_sample, segment);

            avro_value_t avro_value_data;
            avro_value_get_by_name(
                &avro_value_sample, "data", &avro_value_data, NULL);

            avro_value_t avro_value_element;

            // std::vector<std::complex<float>> iqsamples =
            // segment->getIQSamples();

            std::vector<float> iqsamples = segment->getIQ_time_v2();

            for (unsigned int i = 0; i < iqsamples.size(); i = i + 2) {
                size_t new_index;

                float real = iqsamples[i];
                float imag = iqsamples[i + 1];

                // std::complex<float> sample = (segment->getIQSamples()[i]);

                avro_value_append(
                    &avro_value_data, &avro_value_element, &new_index);
                check_i(avro_value_set_float(&avro_value_element, real));

                avro_value_append(
                    &avro_value_data, &avro_value_element, &new_index);
                check_i(avro_value_set_float(&avro_value_element, imag));
            }

            avro_value_decref(&avro_value_element);

            avro_value_write(avro_writer, &avro_value_sample);

            int64_t len_size = avro_writer_tell(avro_writer);
            segment->setAvroBuffer(buf, len_size);

            // segment->setAvroBuffer(buf, buf_size);

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

    mRunning = true;
    SpectrumSegment *segment;

    if (mQueueIn == NULL || mQueueOut == NULL) {
        throw std::logic_error("Queue[IN|OUT] are NULL!");
    }

    unsigned int fft_size = 1 << OpenRFSenseContext::getInstance()->getLog2FftSize();
    unsigned int reduced_fft_size =
        (1 - OpenRFSenseContext::getInstance()->getFreqOverlap()) * (fft_size + 1);
    float freq_res =
        ((float)OpenRFSenseContext::getInstance()->getSamplingRate()) / fft_size;

    const char *json_schema_file = "schemas/rtl-spec.avsc";
    avro_schema_t avro_schema = NULL;
    parseSchema(json_schema_file, avro_schema);

    avro_value_iface_t *avro_iface = avro_generic_class_from_schema(avro_schema);

    while (mRunning) {
        if (mQueueIn && mQueueIn->try_dequeue(segment)) {
            unsigned int buf_size = ((5 * 4 + fft_size * 4) + 3) & ~0x03;
            char *buf = (char *)malloc(buf_size);

            // Create Avro memory writer
            avro_writer_t avro_writer = avro_writer_memory(buf, buf_size);

            // Create a value that is an instance of that schema
            avro_value_t avro_value_sample;
            avro_generic_value_new(avro_iface, &avro_value_sample);

            // Populate metadata
            writeMetadata(avro_value_sample, segment);

            avro_value_t avro_value_config, avro_value_center_freq;
            avro_value_get_by_name(&avro_value_sample, "config", &avro_value_config, NULL);
            avro_value_get_by_name(
                &avro_value_config, "centerFreq", &avro_value_center_freq, NULL);
            avro_value_set_long(
                &avro_value_center_freq, (uint64_t)segment->getCenterFrequency());

            avro_value_t avro_value_data;
            avro_value_get_by_name(
                &avro_value_sample, "data", &avro_value_data, NULL);

            // We need an odd number of bins
            if (reduced_fft_size % 2 == 0)
                reduced_fft_size++;

            avro_value_t avro_value_element;
            unsigned int l = round((fft_size - reduced_fft_size) / 2);

            for (unsigned int i = 0; i < reduced_fft_size; ++i) {
                size_t new_index;
                float s = (segment->getPSDValues()[l + i]);

                avro_value_append(
                    &avro_value_data, &avro_value_element, &new_index);
                if (new_index != i) {
                    fprintf(stderr, "[AvroSerialization] ERROR: Unexpected index.\n");
                    exit(1);
                }
                avro_value_set_float(&avro_value_element, s);
            }

            avro_value_decref(&avro_value_element);
            avro_value_write(avro_writer, &avro_value_sample);

            int64_t len_size = avro_writer_tell(avro_writer);
            segment->setAvroBuffer(buf, len_size);

            mQueueOut->enqueue(segment);

            avro_value_decref(&avro_value_sample);
            avro_writer_free(avro_writer);
        } else
            usleep(1);
    }

    avro_value_iface_decref(avro_iface);
    avro_schema_decref(avro_schema);
}

void AvroSerialization::parseSchema(const char *filename, avro_schema_t &avro_schema) {
    char *json_schema;

    // Read JSON schema from file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(
            stderr, "[AvroSerialization] Failed to open file %s.\n", filename);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    unsigned int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    json_schema = (char *)malloc(file_size + 1);
    size_t read = fread(json_schema, 1, file_size, file);
    if (read != file_size) {
        fprintf(
            stderr, "[AvroSerialization] File %s incompletely read.\n", filename);
        exit(1);
    }
    json_schema[file_size] = '\0';
    fclose(file);

    // Parse JSON schema into C API's internal schema representation
    avro_schema_from_json(json_schema, 0, &avro_schema, NULL);
    if (avro_schema == NULL) {
        fprintf(stderr, "[AvroSerialization] Failed to parse AVRO schema.\n");
        exit(1);
    }
    // Release JSON schema
    free(json_schema);
}

void AvroSerialization::writeMetadata(avro_value_t &base, SpectrumSegment *segment) {
    // Sensing ID and timestamps (required)
    avro_value_t avro_value_lossrate, avro_value_sen_id, avro_value_time,
        avro_value_timesecs, avro_value_timemicrosecs;

    // Write sensor ID, fallback to eth0 mac address string
    char *mac_eth0_dec = 0;
    get_mac_address_eth0(mac_eth0_dec);
    avro_value_get_by_name(&base, "sensorId", &avro_value_sen_id, NULL);
    avro_value_set_string(&avro_value_sen_id, mac_eth0_dec);
    if (!OpenRFSenseContext::getInstance()->getSensorId().empty())
        avro_value_set_string(
            &avro_value_sen_id, OpenRFSenseContext::getInstance()->getSensorId().c_str());

    check_i(avro_value_get_by_name(&avro_value_time, "time", &avro_value_time, NULL));
    avro_value_get_by_name(&avro_value_time, "seconds", &avro_value_timesecs, NULL);
    avro_value_set_long(&avro_value_timesecs, segment->getTimeStamp().tv_sec);

    avro_value_get_by_name(
        &avro_value_time, "microSeconds", &avro_value_timemicrosecs, NULL);
    avro_value_set_int(&avro_value_timemicrosecs, segment->getTimeStamp().tv_nsec);

    avro_value_t avro_value_config, avro_value_frontend_gain, avro_value_center_frequency,
        avro_value_sampling_rate, avro_value_ss_calibrated, avro_value_iq_calibrated,
        avro_value_noise_floor, avro_value_hopping_strategy, avro_value_antenna_gain,
        avro_value_freq_corr, avro_value_antenna_id, avro_value_rfsync,
        avro_value_system_sync, avro_value_extra_conf, avro_value_obfuscation,
        avro_value_branch;

    check_i(avro_value_get_by_name(&base, "config", &avro_value_config, NULL));

    check_i(avro_value_get_by_name(
        &avro_value_config, "hoppingStrategy", &avro_value_hopping_strategy, NULL));
    check_i(avro_value_set_branch(&avro_value_hopping_strategy, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(
        &avro_value_config, "antennaGain", &avro_value_antenna_gain, NULL));
    check_i(avro_value_set_branch(&avro_value_antenna_gain, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(
        &avro_value_config, "frontendGain", &avro_value_frontend_gain, NULL));
    check_i(avro_value_set_branch(&avro_value_frontend_gain, 1, &avro_value_branch));
    check_i(avro_value_set_float(
        &avro_value_branch, OpenRFSenseContext::getInstance()->getGain()));

    check_i(avro_value_get_by_name(
        &avro_value_config, "samplingRate", &avro_value_sampling_rate, NULL));
    check_i(avro_value_set_branch(&avro_value_sampling_rate, 1, &avro_value_branch));
    check_i(avro_value_set_int(
        &avro_value_branch, OpenRFSenseContext::getInstance()->getSamplingRate()));

    check_i(avro_value_get_by_name(
        &avro_value_config, "centerFreq", &avro_value_center_frequency, NULL));
    check_i(avro_value_set_long(
        &avro_value_center_frequency, (uint64_t)segment->getCenterFrequency()));

    check_i(avro_value_get_by_name(
        &avro_value_config, "frequencyCorrectionFactor", &avro_value_freq_corr, NULL));
    check_i(avro_value_set_branch(&avro_value_freq_corr, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(
        &avro_value_config, "antennaId", &avro_value_antenna_id, NULL));
    check_i(avro_value_set_branch(&avro_value_antenna_id, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(
        avro_value_get_by_name(&avro_value_config, "rfSync", &avro_value_rfsync, NULL));
    check_i(avro_value_set_branch(&avro_value_rfsync, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(
        &avro_value_config, "systemSync", &avro_value_system_sync, NULL));
    check_i(avro_value_set_branch(&avro_value_system_sync, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(
        &avro_value_config, "sigStrengthCalibration", &avro_value_ss_calibrated, NULL));
    check_i(avro_value_set_branch(&avro_value_ss_calibrated, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(
        &avro_value_config, "iqBalanceCalibration", &avro_value_iq_calibrated, NULL));
    check_i(avro_value_set_branch(&avro_value_iq_calibrated, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(
        &avro_value_config, "estNoiseFloor", &avro_value_noise_floor, NULL));
    check_i(avro_value_set_branch(&avro_value_noise_floor, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(
        &avro_value_config, "extraConf", &avro_value_extra_conf, NULL));
    check_i(avro_value_set_branch(&avro_value_extra_conf, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(&base, "lossRate", &avro_value_lossrate, NULL));
    check_i(avro_value_set_branch(&avro_value_lossrate, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));

    check_i(avro_value_get_by_name(&base, "obfuscation", &avro_value_obfuscation, NULL));
    check_i(avro_value_set_branch(&avro_value_obfuscation, 0, &avro_value_branch));
    check_i(avro_value_set_null(&avro_value_branch));
}

int AvroSerialization::get_mac_address_eth0(char *mac) {
    int sock;
    const char *ifname = "eth0";
    struct ifreq ifr;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        return -1;
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
        return -2;

    // MAC address hexadecimal
    return sprintf(
        mac, "%02x%02x%02x%02x%02x%02x", (unsigned char)ifr.ifr_hwaddr.sa_data[0],
        (unsigned char)ifr.ifr_hwaddr.sa_data[1],
        (unsigned char)ifr.ifr_hwaddr.sa_data[2],
        (unsigned char)ifr.ifr_hwaddr.sa_data[3],
        (unsigned char)ifr.ifr_hwaddr.sa_data[4],
        (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
}

int AvroSerialization::stop() {
    mRunning = false;
    waitForThread();

    return 1;
}
} // namespace openrfsense
