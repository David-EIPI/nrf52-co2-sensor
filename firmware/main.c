#include "zboss_api.h"
#include "zb_mem_config_custom.h"
#include "zb_error_handler.h"
#include "zigbee_helpers.h"
#include "zb_ha_dimmer_switch.h"
#include "zb_transceiver.h"
#include "zb_common.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrfx_wdt.h"

#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "sensor.h"
#include "battvolt.h"

//#define IEEE_CHANNEL_MASK                   (1l << ZIGBEE_CHANNEL)              /**< Scan only one, predefined channel to find the coordinator. */
//#define LIGHT_SWITCH_ENDPOINT               1                                   /**< Source endpoint used to control light bulb. */
#define ERASE_PERSISTENT_CONFIG             ZB_FALSE                            /**< Do not erase NVRAM to save the network parameters after device reboot or power-off. NOTE: If this option is set to ZB_TRUE then do full device erase for all network devices before running other samples. */

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE to compile light switch (End Device) source code.
#endif

#include "zb_defs.h"

void zb_attr_update_from_scd40(sensor_output_t *data);
void zb_attr_update_battery_voltage(int mv);

ZB_ZCL_DECLARE_AIR_SENSOR_EP(m_first_endpoint, FIRST_ENDPOINT);
ZB_ZCL_DECLARE_CONFIG_EP(m_second_endpoint, SECOND_ENDPOINT);

ZBOSS_DECLARE_DEVICE_CTX_2_EP(m_sensor_device_ctx, m_first_endpoint, m_second_endpoint);

#define MAX_VALID_CO2 5000 /* 5000 is the max value of SCD41, but we will use it as a limit of valid data range */

/**@brief Function to set the Sleeping Mode according to the SLEEPY_ON_BUTTON state.
*/
static void sleepy_device_setup(void)
{
    zb_set_rx_on_when_idle(ERASE_PERSISTENT_CONFIG);

#if ! defined DISABLE_POWER_CONSUMPTION_OPTIMIZATION
    /* If sleepy behaviour is enabled, power off unused RAM to save maximum energy */
    if (ZB_PIBCACHE_RX_ON_WHEN_IDLE() == ZB_FALSE)
    {
        zigbee_power_down_unused_ram();
    }
#endif /* ! defined DISABLE_POWER_CONSUMPTION_OPTIMIZATION */
}

static uint8_t co2_sensor_is_idle = 1;
static bool schedule_call_is_pending = false;
static zb_time_t wait_interval = 0;

static sensor_output_t sensor_data_unknown = SCD40_SENSOR_OUTPUT_UNKNOWN;
static int current_battery_mV = 5000; /* Default value of USB supply */

nrfx_wdt_channel_id m_channel_id;


static void log_float_as_binary(const zb_float32_t *f)
{
#if (NRF_LOG_ENABLED && (NRF_LOG_LEVEL >= NRF_LOG_SEVERITY_DEBUG))
    static char dbg_bin_f[33] = { 0 };

    uint32_t p = f->v;
    unsigned i;
    for (i = 0; i < 32; i++, p <<= 1) {
        dbg_bin_f[i] = p & 0x80000000 ? '1' : '0';
    }
    NRF_LOG_DEBUG("Bin: %s", dbg_bin_f);
#endif
}


/**@brief Read sensors and update Zigbee attributes.
 *
 */
void sensor_loop_helper(uint8_t unused)
{
    (void)unused;

/*  Read battery voltage when CO2 sensor is off.
    CO2 sensor consumes >300mA, which may affect battery voltage.
*/
    if (co2_sensor_is_idle) {
        int batt_v = adc_read();
        if (batt_v >= 0) {
            int mV = adc_to_mv(batt_v);
            NRF_LOG_INFO("Batt V = %d, %d mV", batt_v, mV);
            zb_attr_update_battery_voltage(mV);
            current_battery_mV = mV;
        }
    }

    if (current_battery_mV > 3400) { /* 3.4V, below this voltage LDO dropout becomes too large, causing sensor produce incorrect measurements */
        sensor_loop(&wait_interval, &co2_sensor_is_idle);
    } else {
        zb_attr_update_from_scd40(&sensor_data_unknown);
    }
    schedule_call_is_pending = true;
}


/**@brief Check if a call to sensor_loop_helper() needs to be scheduled and attempt to schedule it.
 *
 */
bool check_pending_sensor_schedule_call(void)
{
    if (schedule_call_is_pending) {
        zb_ret_t zb_err_code = ZB_SCHEDULE_APP_ALARM(sensor_loop_helper, 0, wait_interval);
        if (zb_err_code == RET_OK) {
            schedule_call_is_pending = false;
            return true;
        } else if (zb_err_code == RET_OVERFLOW) {
            NRF_LOG_WARNING("Can not schedule another alarm, queue is full.");
//            ZB_ERROR_CHECK(zb_err_code);
            return false;
        } else {
            ZB_ERROR_CHECK(zb_err_code);
            return false;
        }
    }
    return true;
}

/**@brief Callback function for handling ZCL commands.
 *
 * @param[in]   bufid   Reference to Zigbee stack buffer used to pass received data.
 */
static void zcl_device_cb(zb_bufid_t bufid)
{
    zb_uint8_t cluster_id;
    zb_uint8_t attr_id;
    zb_uint8_t ep_id;
    zb_zcl_device_callback_param_t * p_device_cb_param = ZB_BUF_GET_PARAM(bufid, zb_zcl_device_callback_param_t);

    NRF_LOG_INFO("zcl_device_cb id %hd", p_device_cb_param->device_cb_id);

    /* Set default response value. */
    p_device_cb_param->status = RET_OK;

    switch (p_device_cb_param->device_cb_id)
    {
        case ZB_ZCL_SET_ATTR_VALUE_CB_ID:
            cluster_id = p_device_cb_param->cb_param.set_attr_value_param.cluster_id;
            attr_id    = p_device_cb_param->cb_param.set_attr_value_param.attr_id;
            ep_id      = p_device_cb_param->endpoint;

//            NRF_LOG_INFO("endpoint %d cluster id %hd attr id %hd", ep_id, cluster_id, attr_id);
            if (cluster_id == ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT &&
                attr_id == ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID) {
                zb_float32_t f_value = { .v = p_device_cb_param->cb_param.set_attr_value_param.values.data32 };
                log_float_as_binary(&f_value);
                int32_t i_value = float_to_int32(&f_value);

                NRF_LOG_DEBUG("AO%d=%ld", ep_id, i_value);

                if (i_value >= 0 && i_value < 1000000) { /* Sanity check */
                    if (ep_id == FIRST_ENDPOINT) {
                        sensor_set_calibration_target(i_value);
                    }
                    if (ep_id == SECOND_ENDPOINT) {
                        sensor_request_calibration(i_value);
                    }
                }

            } else {
                p_device_cb_param->status = RET_NOT_IMPLEMENTED;
                NRF_LOG_INFO("Unhandled cluster attribute id: %d 0x%x", cluster_id, attr_id);
            }
            break;
        default:
            p_device_cb_param->status = RET_NOT_IMPLEMENTED;
            break;
    }

    NRF_LOG_INFO("zcl_device_cb status: %hd", p_device_cb_param->status);
}




/**@brief Zigbee stack event handler.
 *
 * @param[in]   bufid   Reference to the Zigbee stack buffer used to pass signal.
 */
void zboss_signal_handler(zb_bufid_t bufid)
{
    zb_zdo_app_signal_hdr_t      * p_sg_p = NULL;
    zb_zdo_app_signal_type_t       sig    = zb_get_app_signal(bufid, &p_sg_p);
//    zb_ret_t                       status = ZB_GET_APP_SIGNAL_STATUS(bufid);
//    zb_ret_t                       zb_err_code;

    if (sig != 22)
        NRF_LOG_INFO("zboss signal %hu", sig);

    switch(sig)
    {
        case 50:
            NRF_LOG_INFO("Intercepting signal 50.");
//            zb_osif_abort();
            break;
        case ZB_BDB_SIGNAL_DEVICE_REBOOT:
            /* fall-through */
        case ZB_BDB_SIGNAL_STEERING:
            /* Call default signal handler. */
            ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

            sensor_loop_helper(0);
            check_pending_sensor_schedule_call();
            break;

        case ZB_COMMON_SIGNAL_CAN_SLEEP:
            /* Zigbee stack can enter sleep state.
             * If the application wants to proceed, it should call zb_sleep_now() function.
             *
             * Note: if the application shares some resources between Zigbee stack and other tasks/contexts,
             *       device disabling should be overwritten by implementing one of the weak functions inside zb_nrf52840_common.c.
             */
            check_pending_sensor_schedule_call();
            zb_sleep_now();
//            ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
            break;

        default:
            /* Call default signal handler. */
            ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));
            break;
    }

    if (bufid)
    {
        zb_buf_free(bufid);
    }
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    ret_code_t err_code;

    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing the nrf hardware watchdog module.
 */
static void wdt_init(void)
{
    nrfx_wdt_config_t config = NRFX_WDT_DEAFULT_CONFIG;
    uint32_t err_code = nrfx_wdt_init(&config, NULL);
    APP_ERROR_CHECK(err_code);
    err_code = nrfx_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrfx_wdt_enable();
}


void zb_attr_update_from_scd40(sensor_output_t *data)
{
/* Feed the dog every time the attributes are updated.
   The WDT module should be configured with a reasonably large value of active CPU time
   so that sensor and Zigbee routines can complete.
*/
    nrfx_wdt_channel_feed(m_channel_id);


    NRF_LOG_INFO("Updating attributes.");
    NRF_LOG_INFO("  CO2 = %u ppm", data->ppm_CO2 == SCD40_CONC_MEASUREMENT_UNKNOWN ? 0 : data->ppm_CO2 );
    NRF_LOG_INFO("  T   = %d C", data->c_temperature == ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_UNKNOWN ? 0 : data->c_temperature);
    NRF_LOG_INFO("  hum = %u %%", data->p_humidity == ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_UNKNOWN ? 0 : data->p_humidity);

    zb_zcl_set_attr_val(FIRST_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
                                     ZB_ZCL_CLUSTER_SERVER_ROLE,
                                     ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID,
                                     (zb_uint8_t *)&data->c_temperature,
                                     ZB_FALSE);


    zb_zcl_set_attr_val(FIRST_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
                                     ZB_ZCL_CLUSTER_SERVER_ROLE,
                                     ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID,
                                     (zb_uint8_t *)&data->p_humidity,
                                     ZB_FALSE);


    static zb_float32_t float_conversion_buf = ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_INVALID;
    if (data->ppm_CO2 != SCD40_CONC_MEASUREMENT_UNKNOWN) {
        concentration_ppm_to_float(data->ppm_CO2, &float_conversion_buf);
    }
    NRF_LOG_DEBUG("ppm=%ld", data->ppm_CO2);
    log_float_as_binary(&float_conversion_buf);

    zb_zcl_set_attr_val(FIRST_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_CO2,
                                     ZB_ZCL_CLUSTER_SERVER_ROLE,
                                     ZB_ZCL_ATTR_CONC_MEASUREMENT_VALUE_ID,
                                     (zb_uint8_t *)&float_conversion_buf,
                                     ZB_FALSE);


    int32_to_float(data->t_calibration, &float_conversion_buf);
    NRF_LOG_DEBUG("tcal=%ld", data->t_calibration);
    log_float_as_binary(&float_conversion_buf);

    zb_zcl_set_attr_val(SECOND_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT,
                                     ZB_ZCL_CLUSTER_SERVER_ROLE,
                                     ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID,
                                     (zb_uint8_t *)&float_conversion_buf,
                                     ZB_FALSE);



}

void zb_attr_update_battery_voltage(int mv)
{
    zb_zcl_set_attr_val(FIRST_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT,
                                     ZB_ZCL_CLUSTER_SERVER_ROLE,
                                     ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_ID,
                                     (zb_uint8_t *)&mv,
                                     ZB_FALSE);
}

void log_chip_info(void)
{
    uint32_t part = NRF_FICR->INFO.PART;
    NRF_LOG_INFO("chip part no = 0x%x", part);
    NRF_LOG_FLUSH();
    uint32_t var = NRF_FICR->INFO.VARIANT;
    NRF_LOG_INFO("chip variant = %c%c%c%c", var&0xff, (var>>8)&0xff, (var>>16)&0xff, (var>>24)&0xff);
    NRF_LOG_FLUSH();
    uint32_t pack = NRF_FICR->INFO.PACKAGE;
    NRF_LOG_INFO("chip package = 0x%x", pack);
    NRF_LOG_FLUSH();
}

void led_blink(int count)
{
    int i = 0;

    for (i = 0; i < count; i++) {
        nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0, 15));
        nrf_delay_ms(250);
        nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 15));
        nrf_delay_ms(250);
    }

}

/**@brief Function for application main entry.
 */
int main(void)
{
    zb_ret_t       zb_err_code;
    zb_ieee_addr_t ieee_addr;

    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 15));

    /* Initialize loging system and GPIOs. */
    log_init();
    log_chip_info();

    sensor_init (zb_attr_update_from_scd40);

    calibration_test();
    zb_trans_set_tx_power(-8);

    /* Set Zigbee stack logging level and traffic dump subsystem. */
    ZB_SET_TRACE_LEVEL(ZIGBEE_TRACE_LEVEL);
    ZB_SET_TRACE_MASK(ZIGBEE_TRACE_MASK);
    ZB_SET_TRAF_DUMP_OFF();

    /* Indicate boot progress */
    led_blink(2);

    /* Initialize Zigbee stack. */
    ZB_INIT("air_sensor");

    /* Indicate successful initialization of Zigbee subsystem */
    led_blink(2);

    /* Set device address to the value read from FICR registers. */
    zb_osif_get_ieee_eui64(ieee_addr);
    zb_set_long_address(ieee_addr);

    zb_set_network_ed_role(IEEE_CHANNEL_MASK);
    zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);

    zb_set_ed_timeout(ED_AGING_TIMEOUT_64MIN);
    zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(15000));
    sleepy_device_setup();

    /* Register callback for handling ZCL commands. */
    ZB_ZCL_REGISTER_DEVICE_CB(zcl_device_cb);

    /* Register sensor device context (endpoints). */
    ZB_AF_REGISTER_DEVICE_CTX(&m_sensor_device_ctx);

    /** Start Zigbee Stack. */
    zb_err_code = zboss_start_no_autostart();
    ZB_ERROR_CHECK(zb_err_code);

    /* Start the watchdog just before the main loop. */
    wdt_init();

    /* Indicate successful initialization of all modules */
    led_blink(2);

    while(1)
    {
        zboss_main_loop_iteration();
        UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
    }
}


/**
 * @}
 */
