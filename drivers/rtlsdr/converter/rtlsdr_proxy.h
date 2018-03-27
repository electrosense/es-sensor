/*
 * rtlsdr.h
 *
 *  Created on: Sep 11, 2017
 *      Author: bertold
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdint.h>
#include <rtl-sdr.h>

#ifndef RTLSDR_H_
#define RTLSDR_H_

/* We only need a few functions */
typedef int (*base_rtlsdr_open_t)(rtlsdr_dev_t **dev, uint32_t index);
base_rtlsdr_open_t base_rtlsdr_open;

typedef int (*base_rtlsdr_set_center_freq_t)(rtlsdr_dev_t *dev, uint32_t freq);
base_rtlsdr_set_center_freq_t base_rtlsdr_set_center_freq;

typedef uint32_t (*base_rtlsdr_get_center_freq_t)(rtlsdr_dev_t *dev);
base_rtlsdr_get_center_freq_t base_rtlsdr_get_center_freq;

typedef int (*base_rtlsdr_set_center_freq64_t)(rtlsdr_dev_t *dev, uint64_t freq);
base_rtlsdr_set_center_freq64_t base_rtlsdr_set_center_freq64;

typedef uint64_t (*base_rtlsdr_get_center_freq64_t)(rtlsdr_dev_t *dev);
base_rtlsdr_get_center_freq64_t base_rtlsdr_get_center_freq64;

typedef int (*base_rtlsdr_read_sync_t)(rtlsdr_dev_t *dev, void *buf, int len, int *n_read);
base_rtlsdr_read_sync_t base_rtlsdr_read_sync;

typedef int (*base_rtlsdr_wait_async_t)(rtlsdr_dev_t *dev, rtlsdr_read_async_cb_t cb, void *ctx);
base_rtlsdr_wait_async_t base_rtlsdr_wait_async;

typedef int (*base_rtlsdr_read_async_t)(rtlsdr_dev_t *dev,
										rtlsdr_read_async_cb_t cb,
										void *ctx,
										uint32_t buf_num,
										uint32_t buf_len);
base_rtlsdr_read_async_t base_rtlsdr_read_async;


#endif /* RTLSDR_H_ */
