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
 * 	Authors:
 * 	    Roberto Calvo Palomino <roberto.calvo@imdea.org>
 *
 */


#include "rtlsdrDriver.h"

namespace electrosense {


rtlsdrDriver::rtlsdrDriver()
{

    mQueueOut = new ReaderWriterQueue<SpectrumSegment*>(100);
}


int rtlsdrDriver::open(std::string device)
{
    // rtlsdr lib expects to have integer id for identifying the device
    mDeviceId = std::stoi(device);


    int device_index=mDeviceId;

    int n_rtlsdr=rtlsdr_get_device_count();

    if (n_rtlsdr==0) {
        std::cerr << "* Error: no RTL-SDR USB devices found" << std::endl;
        throw std::logic_error("Fatal Error");
	}

    // Choose which device to use
    if ((n_rtlsdr==1)&&(device_index==-1)) {
        device_index=0;
    }
    if ((device_index<0)||(device_index>=n_rtlsdr)) {
        std::cerr << "Error: must specify which USB device to use with --device-index" << std::endl;
        std::cerr << "Found the following USB devices:" << std::endl;
        char vendor[256],product[256],serial[256];
        for (int t=0;t<n_rtlsdr;t++) {
            rtlsdr_get_device_usb_strings(t,vendor,product,serial);
            std::cerr << "Device index " << t << ": [Vendor: " << vendor << "] [Product: " << product << "] [Serial#: " << serial << "]" << std::endl;
        }
    }

    // Open
	if (rtlsdr_open(&mDevice,device_index)<0) {
		std::cerr << "ERROR: unable to open RTLSDR device" << std::endl;
        throw std::logic_error("Fatal Error");
	}

    int samplingRate = ElectrosenseContext::getInstance()->getSamplingRate();

    // Sampling frequency
	if (rtlsdr_set_sample_rate(mDevice,samplingRate)<0) {
		std::cerr << "ERROR: unable to set sampling rate to " << samplingRate << std::endl;
        throw std::logic_error("Fatal Error");
	}

    int frequency = ElectrosenseContext::getInstance()->getMinFreq();

    if (rtlsdr_set_center_freq(mDevice,frequency)<0)
	{
		std::cerr << "ERROR: unable to set frequency to" << frequency << std::endl;
        throw std::logic_error("Fatal Error");
    }

    int* gains;
    int count = rtlsdr_get_tuner_gains(mDevice, NULL);
    if (count > 0 ) {
        gains = (int*) malloc(sizeof(int) * count);
        count = rtlsdr_get_tuner_gains(mDevice, gains);
        std::cout << "Gain available: ";
        for (int i=0; i<count; i++)
            std::cout  << gains[i] << " , ";

        std::cout << std::endl;
        free(gains);
    }

    int gain = ElectrosenseContext::getInstance()->getGain();

    int r = rtlsdr_set_tuner_gain_mode(mDevice, 1);
	if(r < 0) {
		std::cerr << "ERROR: Failed to enable manual gain mode" << std::endl;
        throw std::logic_error("Fatal Error");
	}
	r = rtlsdr_set_tuner_gain(mDevice, gain*10);
	if(r < 0) {
		std::cerr << "ERROR: Failed to set manual tuner gain" << std::endl;
        throw std::logic_error("Fatal Error");
	}
	else {
        int g = rtlsdr_get_tuner_gain(mDevice);
        std::cout << "Gain set to " << g/10 << std::endl;
    }

	// Reset the buffer
	if (rtlsdr_reset_buffer(mDevice)<0) {
		std::cerr << "Error: unable to reset RTLSDR buffer" << std::endl;
	}


    std::cout << "[*] Initializing dongle with following configuration: " << std::endl;
	std::cout << "\t Center Frequency: " << frequency << " Hz" << std::endl;
	std::cout << "\t Sampling Rate: " << samplingRate << " samples/sec" << std::endl;
    std::cout << "\t Gain: " << gain << " dB" << std::endl;


    return 1;

}


int rtlsdrDriver::close ()
{

    return 1;
}


void rtlsdrDriver::run ()
{
    const int BULK_TRANSFER_MULTIPLE = 512;

    mRunning = true;
    std::cout << "rtlsdrDriver::run" << std::endl;

    mSeqHopping = new SequentialHopping();
    unsigned int center_freq=0, previous_freq=0, fft_size=0, slen=0;

    while (mRunning)
    {

        // Introduce here the concept of segment per band (before jumping).

        center_freq = mSeqHopping->nextHop();

        if (previous_freq != center_freq)
		{
            previous_freq = center_freq;
			int r = rtlsdr_set_center_freq(mDevice, center_freq);
            if (r!=0)
                std::cerr << "Error: unable to set center frequency" << std::endl;

            // Reset the buffer
            if (rtlsdr_reset_buffer(mDevice)<0) {
                std::cerr << "Error: unable to reset RTLSDR buffer" << std::endl;
            }

            //unsigned int freq = rtlsdr_get_center_freq(mDevice);
            //std::cout << "Center Freq: " << freq << " Hz" << std::endl;


        }

        uint8_t *iq_buf = NULL;
        unsigned int current_fft_size = 1<<ElectrosenseContext::getInstance()->getLog2FftSize();


        if (fft_size != current_fft_size)
        {

            fft_size = current_fft_size;

            slen = ((current_fft_size-ElectrosenseContext::getInstance()->getSoverlap()) *
                    ElectrosenseContext::getInstance()->getAvgFactor()+ElectrosenseContext::getInstance()->getSoverlap())*2;


            // NOTE: libusb_bulk_transfer for RTL-SDR seems to crash when not reading multiples of 512 (BULK_TRANSFER_MULTIPLE)
            if(slen % BULK_TRANSFER_MULTIPLE != 0)
                slen = slen + (BULK_TRANSFER_MULTIPLE - (slen % BULK_TRANSFER_MULTIPLE));

            iq_buf = (uint8_t *) realloc(iq_buf,slen*sizeof(uint8_t));

        }
        else
        {
            iq_buf = (uint8_t *) realloc(iq_buf,slen*sizeof(uint8_t));
        }

        int n_read;
        struct timespec current_time;
	    clock_gettime(CLOCK_REALTIME, &current_time);

        int r = rtlsdr_read_sync(mDevice, iq_buf, slen, &n_read);
        if(r != 0 || (unsigned int)n_read != slen) fprintf(stderr, "WARNING: Synchronous read failed.\n");


        std::vector<std::complex<float>> iq_vector;

        for (unsigned int i=0; i<ElectrosenseContext::getInstance()->getAvgFactor(); i++) {

            iq_vector.clear();

            for (unsigned int j = 0; j < current_fft_size*2; j = j + 2) {

                // Every segment overlaps getSoverlap() samples in time domain.

                iq_vector.push_back( std::complex<float>(
                        iq_buf[j+i*(current_fft_size-ElectrosenseContext::getInstance()->getSoverlap())*2],
                        iq_buf[j+1+i*(current_fft_size-ElectrosenseContext::getInstance()->getSoverlap())*2] ));

            }

            //TODO: Id should be the ethernet MAC

            SpectrumSegment *segment = new SpectrumSegment(-1000, current_time, center_freq,
                                                           ElectrosenseContext::getInstance()->getSamplingRate(),
                                                           iq_vector);
            mQueueOut->enqueue(segment);

        }

        //break;

    }

    delete(mSeqHopping);

}



int rtlsdrDriver::stop()
{
    mRunning = false;
    waitForThread();

    rtlsdr_close(mDevice);
	mDevice = NULL;

    return 1;
}


}
