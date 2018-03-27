#include "rtlsdr_proxy.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "converter.h"
static converter converterDriver;
uint64_t currentRtlFreq;
uint64_t currentTunerFreq;
bool useInversion;

int rtlsdr_open(rtlsdr_dev_t **dev, uint32_t index){
#define DLSYM_INIT(type, vname, fname) do{ vname = (type)dlsym(RTLD_NEXT, fname); \
			if(!vname) printf("Failed to load %s\n", fname);} while(0)

	/* Here we look up all the other functions */
	DLSYM_INIT(base_rtlsdr_open_t, base_rtlsdr_open, "rtlsdr_open");
	DLSYM_INIT(base_rtlsdr_set_center_freq_t, base_rtlsdr_set_center_freq, "rtlsdr_set_center_freq");
	DLSYM_INIT(base_rtlsdr_get_center_freq_t, base_rtlsdr_get_center_freq, "rtlsdr_get_center_freq");
	DLSYM_INIT(base_rtlsdr_set_center_freq64_t, base_rtlsdr_set_center_freq64, "rtlsdr_set_center_freq64");
	DLSYM_INIT(base_rtlsdr_get_center_freq64_t, base_rtlsdr_get_center_freq64, "rtlsdr_get_center_freq64");
	DLSYM_INIT(base_rtlsdr_read_sync_t, base_rtlsdr_read_sync, "rtlsdr_read_sync");
	DLSYM_INIT(base_rtlsdr_wait_async_t, base_rtlsdr_wait_async, "rtlsdr_wait_async");
	DLSYM_INIT(base_rtlsdr_read_async_t, base_rtlsdr_read_async, "rtlsdr_read_async");
#undef DLSYM_INIT

	converterDriver.portPath = getenv("CONVERTER_PORT");
	if(!converterDriver.portPath) {
		converterDriver.portPath = "/dev/esenseconv";
	}
	if(!converterInit(&converterDriver)){
		perror("Failed to open converter");
		return -1;
	}

	printf("Bertold: converter proxy ready\n");
	return base_rtlsdr_open(dev, index);
}

int rtlsdr_set_center_freq64(rtlsdr_dev_t *dev, uint64_t freq){

	uint64_t newFreq;
	bool mustInvert;

	/* Electrosense only works up to 2GHz, so time multiplex */
	time_t tt;
	time ( &tt );
	struct tm*tm = gmtime ( &tt );


	/*	
	uint64_t currentShift;	
	char* forceShift = getenv("BAND");
	if(!forceShift){
		currentShift = (tm->tm_min / 20);
	}else{
		currentShift = strtoul(forceShift, NULL, 10);
	}
*/
	/* Avoid errors */
//	currentTunerFreq = freq;
	
	/* Freq is now from 0-2GHz */
//	freq = freq % 2000000000;
	/* And now shifted */
//	freq += 2000000000 * currentShift;

	currentTunerFreq = freq;

	if(!converterTune(&converterDriver, freq/1000, &newFreq, &mustInvert)){
		return -1;
	}

	printf("%u:%u:%u (sector ): Tuning to %llu kHz, receiving on %llu kHz", tm->tm_hour, tm->tm_min, tm->tm_sec,freq/1000, newFreq);
	if(mustInvert){
		printf(" with spectrum inversion.\n");
	}else{
		printf(".\n");
	}

	useInversion = mustInvert;

	if(newFreq == currentRtlFreq){
		return 0;
	}

	currentRtlFreq = newFreq;

	return base_rtlsdr_set_center_freq(dev, newFreq * 1000);
}

uint64_t rtlsdr_get_center_freq64(rtlsdr_dev_t *dev){
	return currentTunerFreq;
}

int rtlsdr_set_center_freq(rtlsdr_dev_t *dev, uint32_t freq){
	return rtlsdr_set_center_freq64(dev, freq);
}

uint32_t rtlsdr_get_center_freq(rtlsdr_dev_t *dev){
	return rtlsdr_get_center_freq64(dev);
}

void invertSpectrum(uint8_t* buf, int bufLen){
	for(unsigned int i = 0; i<bufLen; i+= 2){
		buf[i] = 255-buf[i];
	}
}

int rtlsdr_read_sync(rtlsdr_dev_t *dev, void *buf, int len, int *n_read){
	int retVal = base_rtlsdr_read_sync(dev, buf, len, n_read);

	if(useInversion){
		invertSpectrum(buf, *n_read);
	}

	return retVal;
}

rtlsdr_read_async_cb_t origCallback;

void proxyCallback(unsigned char *buf, uint32_t len, void *ctx){
	if(useInversion){
		invertSpectrum(buf, len);
	}

	origCallback(buf, len, ctx);
}

int rtlsdr_wait_async(rtlsdr_dev_t *dev, rtlsdr_read_async_cb_t cb, void *ctx){
	origCallback = cb;
	return base_rtlsdr_wait_async(dev, proxyCallback, ctx);
}


int rtlsdr_read_async(rtlsdr_dev_t *dev,
					  rtlsdr_read_async_cb_t cb,
					  void *ctx,
					  uint32_t buf_num,
					  uint32_t buf_len){

	origCallback = cb;

	return base_rtlsdr_read_async(dev, proxyCallback, ctx, buf_num, buf_len);
}
