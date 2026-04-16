#ifndef _BATTVOLT_H_
#define _BATTVOLT_H_
#include "nrfx_saadc.h"

nrfx_err_t adc_init(void);
int adc_read(void);
int adc_to_mv(int adc_val);

int calibration_test(void);

#endif
