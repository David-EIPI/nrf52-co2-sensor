#ifndef _SENSOR_H_
#define _SENSOR_H_

#include "zboss_api.h"

typedef struct {
    uint32_t ppm_CO2;       /* CO2 concentration, in parts-per-million */
    int32_t c_temperature;  /* temperature x 10, C */
    uint32_t p_humidity;    /* humidity x 10, %*/
    uint32_t t_calibration; /* Calibration progress, s*/
} sensor_output_t;

typedef void (*data_cb_t)(sensor_output_t *data);

void sensor_init(data_cb_t cb);
//void sensor_loop_helper(zb_uint8_t position);
void sensor_loop(zb_time_t *wait_interval, uint8_t *is_idle);

void sensor_request_calibration(uint32_t calib_time);
void sensor_set_calibration_target(uint32_t target);

uint8_t sensirion_common_generate_crc(const uint8_t* data, uint16_t count);

void test_twi_xfer(void);
#endif
