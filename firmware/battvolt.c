#include "nrfx_saadc.h"
#include "nrf_log.h"
#include "nrf_delay.h"

#include "battvolt.h"

//#define USE_VDDH5

static nrfx_saadc_config_t bv_saadc_config = NRFX_SAADC_DEFAULT_CONFIG;



#ifdef USE_VDDH5
static const nrf_saadc_input_t battery_channel = SAADC_CH_PSELP_PSELP_VDDHDIV5;
#else
static const nrf_saadc_input_t battery_channel = NRF_SAADC_INPUT_AIN7;
//static const nrf_saadc_input_t battery_channel = NRF_SAADC_INPUT_AIN0;
#endif

static nrf_saadc_channel_config_t bv_channel_config =
{
    .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
    .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
#ifdef USE_VDDH5
    .gain       = NRF_SAADC_GAIN1_2,
#else
    .gain       = NRF_SAADC_GAIN1_5,
#endif
    .reference  = NRF_SAADC_REFERENCE_INTERNAL,
//    .reference  = NRF_SAADC_REFERENCE_VDD4,
    .acq_time   = NRF_SAADC_ACQTIME_40US,
    .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
    .burst      = NRF_SAADC_BURST_DISABLED,
    .pin_p      = battery_channel,
    .pin_n      = NRF_SAADC_INPUT_DISABLED
};


static uint8_t bv_channel = 0;
static nrf_saadc_value_t bv_value = -1;
static int calibration_offset = 0;

/*@brief Initialize ADC to read battery voltage.
 On Promicro board this will be available only via VDDH channel.
 ADC is initialized in blocking mode, as it will be used when
 CO2 sensor is not active.
 */
nrfx_err_t adc_init(void)
{
    nrfx_err_t res = nrfx_saadc_init(&bv_saadc_config, NULL);
    if (NRFX_SUCCESS == res) {
        res = nrfx_saadc_channel_init(bv_channel, &bv_channel_config);
        if (NRFX_SUCCESS == res) {
            NRF_LOG_DEBUG("ADC channel initialized");
        }
    }
    return res;
}

int adc_to_mv(int adc_val)
{
#ifdef USE_VDDH5
    int mV = (adc_val * 600 * 2 * 5) >> 14;
#else
    int mV = (adc_val * 4468) >> 14;
#endif
//    DIODE_FWD_VOLT_DROP_MILLIVOLTS
    return mV;
}

int adc_read(void)
{
    nrfx_err_t res = adc_init();

    int ret_v = 0;
    int i = 0;
    while (i < 256 && res == NRFX_SUCCESS) {
        i += 1;
        res = nrfx_saadc_sample_convert(bv_channel, &bv_value);
        if (NRFX_SUCCESS != res)
            break;
        ret_v += bv_value;
    }
    ret_v /= i;

//    res = nrfx_saadc_sample_convert(bv_channel, &bv_value);

    if (NRFX_SUCCESS != res) {
        NRF_LOG_DEBUG("ADC read error: %d", res);
        ret_v = -1;
    } else {
        NRF_LOG_DEBUG("ADC read value: %d", ret_v);
        ret_v -= calibration_offset;
    }

    nrfx_saadc_uninit();

    return ret_v;
}


static nrf_saadc_channel_config_t cal_channel_config =
{
    .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
    .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
    .gain       = NRF_SAADC_GAIN1_6,
    .reference  = NRF_SAADC_REFERENCE_INTERNAL,
//    .reference  = NRF_SAADC_REFERENCE_VDD4,
    .acq_time   = NRF_SAADC_ACQTIME_40US,
    .mode       = NRF_SAADC_MODE_DIFFERENTIAL,
    .burst      = NRF_SAADC_BURST_DISABLED,
    .pin_p      = battery_channel,
    .pin_n      = battery_channel,
};

int calibration_test(void)
{
    int ret_v = 0;
    nrfx_err_t res = nrfx_saadc_init(&bv_saadc_config, NULL);
    if (NRFX_SUCCESS == res) {
        res = nrfx_saadc_channel_init(bv_channel, &cal_channel_config);

        if (NRFX_SUCCESS == res) {
            int i;
            for (i = 0; i < 50 && res == NRFX_SUCCESS; i++) {
                res = nrfx_saadc_sample_convert(bv_channel, &bv_value);
                if (NRFX_SUCCESS != res)
                    break;
                ret_v += bv_value;
            }
            ret_v /= i;
            calibration_offset = ret_v;
            NRF_LOG_DEBUG("Calibration offset: %d", ret_v);
        }

        nrfx_saadc_uninit();
    }
    return ret_v;
}

