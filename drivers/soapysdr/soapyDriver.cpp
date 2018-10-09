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


#include "soapyDriver.h"
#include <pthread.h>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Constants.h>

namespace electrosense {


soapyDriver::soapyDriver() {

    mQueueOut = new ReaderWriterQueue<SpectrumSegment*>(100);
    mConverterEnabled = false;
}


int soapyDriver::open(std::string device) {
    // rtlsdr lib expects to have integer id for identifying the device

    int device_index = -1;
		SoapySDR::KwargsList list = SoapySDR::Device::enumerate(device);
    int n_rtlsdr = list.size();

    if (n_rtlsdr==0) {
        std::cerr << "* Error: no RTL-SDR USB devices found" << std::endl;
        throw std::logic_error("Fatal Error");
	}

    // Choose which device to use
    if ((n_rtlsdr==1)) {
        device_index=0;
        std::cout << "Device index " << device_index << ": [Device: " << list[device_index]["device"] << "] [Product: " << list[device_index]["driver"] << "] [URI: " << list[device_index]["uri"] << "]" << std::endl;
    }
    if ((device_index<0)||(device_index>=n_rtlsdr)) {
        std::cerr << "Error: must specify which USB device to use with --device-index" << std::endl;
        std::cerr << "Found the following USB devices:" << std::endl;
        for (int t=0;t<n_rtlsdr;t++) {
            std::cerr << "Device index " << t << ": [Device: " << list[t]["device"] << "] [Product: " << list[t]["driver"] << "] [URI: " << list[t]["uri"] << "]" << std::endl;
        }
    }

    // Open
		mDevice = SoapySDR::Device::make(list[device_index]);
        if (mDevice==NULL) {
        	std::cerr << "ERROR: unable to open SDR device" << std::endl;
        throw std::logic_error("Fatal Error");
        }

    int samplingRate = ElectrosenseContext::getInstance()->getSamplingRate();

    // Sampling frequency
		mDevice->setSampleRate(SOAPY_SDR_RX,0,samplingRate);
		mDevice->setBandwidth(SOAPY_SDR_RX,0,samplingRate);
    //if (rtlsdr_set_sample_rate(mDevice,samplingRate)<0) {
    //    	std::cerr << "ERROR: unable to set sampling rate to " << samplingRate << std::endl;
    //    throw std::logic_error("Fatal Error");
    //}

	  int frequency = 24e6; // default value
		mDevice->setFrequency(SOAPY_SDR_RX,0,frequency);
//  if (rtlsdr_set_center_freq(mDevice,frequency)<0)
//      {
//      	std::cerr << "ERROR: unable to set frequency to" << frequency << std::endl;
//      throw std::logic_error("Fatal Error");
//  }

//  int* gains;
//  int count = rtlsdr_get_tuner_gains(mDevice, NULL);
//  if (count > 0 ) {
//      gains = (int*) malloc(sizeof(int) * count);
//      count = rtlsdr_get_tuner_gains(mDevice, gains);
//      std::cout << "Gain available: ";
//      for (int i=0; i<count; i++)
//          std::cout  << gains[i] << " , ";

//      std::cout << std::endl;
//      free(gains);
//  }

    double gain = ElectrosenseContext::getInstance()->getGain();
		mDevice->setGainMode(SOAPY_SDR_RX,0,false);

//  int r = rtlsdr_set_tuner_gain_mode(mDevice, 1);
//      if(r < 0) {
//      	std::cerr << "ERROR: Failed to enable manual gain mode" << std::endl;
//      throw std::logic_error("Fatal Error");
//      }
//      r = rtlsdr_set_tuner_gain(mDevice, gain*10);
		mDevice->setGain(SOAPY_SDR_RX,0,gain);
//      if(r < 0) {
//      	std::cerr << "ERROR: Failed to set manual tuner gain" << std::endl;
//      throw std::logic_error("Fatal Error");
//      }
//      else {
		gain = mDevice->getGain(SOAPY_SDR_RX,0);
//      int g = rtlsdr_get_tuner_gain(mDevice);
//      std::cout << "Gain set to " << g/10 << std::endl;
//  }

//      // Reset the buffer
//      if (rtlsdr_reset_buffer(mDevice)<0) {
//      	std::cerr << "Error: unable to reset RTLSDR buffer" << std::endl;
//      }


    std::cout << "[*] Initializing dongle with following configuration: " << std::endl;
    std::cout << "\t Center Frequency: " << frequency << " Hz" << std::endl;
    std::cout << "\t Sampling Rate: " << samplingRate << " samples/sec" << std::endl;
    std::cout << "\t Gain: " << gain << " dB" << std::endl;

//  // Check if the converter is present 

//  mConverterDriver.portPath = new char[CONVERTER_PATH.size() + 1];
//  std::copy(CONVERTER_PATH.begin(), CONVERTER_PATH.end(), mConverterDriver.portPath);
//  mConverterDriver.portPath[CONVERTER_PATH.size()] = '\0';

//  if(!converterInit(&mConverterDriver)){
//      std::cerr << "Warning: Failed to open the converter" << std::endl;
//      //throw std::logic_error("Failed to open the converter");
//  }
//  else {
//      std::cout << "Converter has been detected properly" << std::endl;
//      mConverterEnabled = true;
//  }

//  delete[] mConverterDriver.portPath;


  	return 1;

}


int soapyDriver::close () {
    return 1;
}


void timespec_diff(struct timespec *start, struct timespec *stop,
                   struct timespec *result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }

    return;
}

typedef struct {
		soapyDriver * dev;
} callback_package_t;


//static void capbuf_rtlsdr_callback( unsigned char * buf, uint32_t len, void * ctx) {
//
//    struct sched_param param;
//	param.sched_priority = 20;
//	pthread_setschedparam (pthread_self ( ) ,SCHED_FIFO,&param ) ;
//
//    struct timespec current_time;
//	clock_gettime(CLOCK_REALTIME, &current_time);
//
//
//    // Getting parameters
//    callback_package_t * cp_p=(callback_package_t *)ctx;
//	callback_package_t & cp=*cp_p;
//    std::vector<std::complex<float>> capbuf_raw_p = *cp.buf;
//    uint64_t center_freq = cp.center_frequency;
//    ReaderWriterQueue< SpectrumSegment*> *queue = cp.queue;
////    rtlsdr_dev_t * dev=cp.dev;
//
//    // we set init time to current time only in the first execution
//    if (cp.init_time.tv_sec == 0)
//        cp.init_time = current_time;
//
//    for (uint32_t t=0;t<len;t=t+2) {
//        capbuf_raw_p.push_back( std::complex<float>(buf[t] ,buf[t+1] ));
//	}
//
//    //std::cout << "[*] Sending segment " << current_time.tv_sec << "." << current_time.tv_nsec << std::endl;
//    SpectrumSegment *segment = new SpectrumSegment(-1000, current_time, center_freq,
//                                                   ElectrosenseContext::getInstance()->getSamplingRate(),
//                                                   capbuf_raw_p);
//
//    queue->enqueue(segment);
//    capbuf_raw_p.clear();
//
//    *cp.samples_read = *cp.samples_read + len;
//
//    struct timespec diff;
//	timespec_diff(&cp.init_time, &current_time, &diff);
//
//	if (cp.duration != 0 && (diff.tv_sec >= cp.duration))
//	{
////		rtlsdr_cancel_async(dev);
//        pthread_exit(NULL);
//		return;
//	}
//}

bool soapyDriver::isRunning () {
    return mRunning;
}

static void *aux_thread (void *arg) {
    soapyDriver *sd_t = (soapyDriver *) arg;
		sd_t->SyncSampling();
    pthread_exit(NULL);
}


void soapyDriver::run () {
    mRunning = true;
//        SyncSampling();
        pthread_t myth;
        pthread_create ( &myth, NULL, aux_thread, (void *)this );
        pthread_join(myth,NULL);
    mRunning = false;
}

void soapyDriver::SyncSampling() {

    const int BULK_TRANSFER_MULTIPLE = 512;
    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    std::cout << "soapyDriver::run" << std::endl;

    mSeqHopping = new SequentialHopping();
    uint64_t center_freq=0, previous_freq=0, fft_size=0, slen=0;

		std::complex<float> *iq_buf = NULL;

    while (mRunning)
    {

        // Introduce here the concept of segment per band (before jumping).

        center_freq = mSeqHopping->nextHop();

	      if (previous_freq != center_freq)
				{
          	previous_freq = center_freq;

//          // Native RTL-SDR

//              int r = 0;//rtlsdr_set_center_freq(mDevice, center_freq);
						mDevice->setFrequency(SOAPY_SDR_RX,0,center_freq);	
//              if (r != 0) {
//                  std::cerr << "Error: unable to set center frequency" << std::endl;
//                  mRunning = false;
//              }

//              // Reset the buffer
						if (mStream != 0)
						{
							mDevice->deactivateStream(mStream,0,0);
							mDevice->closeStream(mStream);
						}
						mStream=mDevice->setupStream(SOAPY_SDR_RX,"CF32");
						mDevice->activateStream(mStream);		
//              if (false){//rtlsdr_reset_buffer(mDevice)<0) 
//                  std::cerr << "Error: unable to reset RTLSDR buffer" << std::endl;
//                  mRunning = false;
//              }



        }

        unsigned int current_fft_size = 1<<ElectrosenseContext::getInstance()->getLog2FftSize();

        if (fft_size != current_fft_size)
        {

            fft_size = current_fft_size;

            slen = ((current_fft_size-ElectrosenseContext::getInstance()->getSoverlap()) *
                    ElectrosenseContext::getInstance()->getAvgFactor()+ElectrosenseContext::getInstance()->getSoverlap())*2;


            if(slen % BULK_TRANSFER_MULTIPLE != 0)
                slen = slen + (BULK_TRANSFER_MULTIPLE - (slen % BULK_TRANSFER_MULTIPLE));

            iq_buf = (std::complex<float> *) realloc(iq_buf,slen*sizeof(std::complex<float>));

        }
				std::vector<void *> iqbufs(1); 
				iqbufs[0] = iq_buf;

        struct timespec current_time;
      	clock_gettime(CLOCK_REALTIME, &current_time);

				int flags;
				long long timeNs;
        int r = mDevice->readStream(mStream,iqbufs.data(),slen,flags,timeNs,100000);
        if(r <= 0 ){ 
            fprintf(stderr, "WARNING: Synchronous read failed.\n");
						std::cout << r <<  " " << slen << std::endl;
            mRunning = false;
        }

        //    for(int i = 0; i<n_read; i+= 2){
        //        iq_buf[i] = 255-iq_buf[i];
        //    }
        //}


        std::vector<std::complex<float>> iq_vector;

      for (unsigned int i=0; i<ElectrosenseContext::getInstance()->getAvgFactor(); i++) {

            iq_vector.clear();

              for (unsigned int j = 0; j < current_fft_size*2; j ++) {

                // Every segment overlaps getSoverlap() samples in time domain.

                iq_vector.push_back( 
                        iq_buf[j+i*(current_fft_size-ElectrosenseContext::getInstance()->getSoverlap())*2]);

            }

            //TODO: Id should be the ethernet MAC

            SpectrumSegment *segment = new SpectrumSegment(-1000, current_time, center_freq,
                                                           ElectrosenseContext::getInstance()->getSamplingRate(),
                                                           iq_vector);
            mQueueOut->enqueue(segment);

        }
				struct timespec diff;
				timespec_diff(&start_time, &current_time, &diff);
				if (diff.tv_sec > ElectrosenseContext::getInstance()->getMonitorTime() && 
							ElectrosenseContext::getInstance()->getMonitorTime()!= 0)
				{
					std::cout << "sampling done" << std::endl;
					mRunning = false;
				}
    }

    delete(mSeqHopping);

}




int soapyDriver::stop()
{
    mRunning = false;
    waitForThread();

    // Workaround: RPi does not work properly the cancellation of the async task
    if (ElectrosenseContext::getInstance()->getPipeline().compare("PSD") ==0 ) {
//        rtlsdr_close(mDevice);
				SoapySDR::Device::unmake(mDevice);
        mDevice = NULL;
    }

    return 1;
}
}
