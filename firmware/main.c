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
#include "nrf_power.h"
#include "nrf_drv_clock.h"

#include "app_timer.h"
#include "app_scheduler.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "zigbee_dfu_transport.h"
#include "nrf_dfu_settings.h"


#include "app_dfu_abort.h"
#include "app_dfu_finalize.h"
#include "sensor.h"
#include "battvolt.h"
#include "liion.h"

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
ZB_HA_DECLARE_OTA_UPGRADE_CLIENT_EP(ota_upgrade_client_ep, OTA_ENDPOINT, ota_upgrade_client_clusters);

ZBOSS_DECLARE_DEVICE_CTX_3_EP(m_sensor_device_ctx, m_first_endpoint, m_second_endpoint, ota_upgrade_client_ep);

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
        NRF_LOG_INFO("RAM power-down optimization skipped");
        NRF_LOG_FLUSH();
    }
#endif /* ! defined DISABLE_POWER_CONSUMPTION_OPTIMIZATION */
}

static uint8_t co2_sensor_is_idle = 1;
static bool schedule_call_is_pending = false;
static zb_time_t wait_interval = 0;

static sensor_output_t sensor_data_unknown = SCD40_SENSOR_OUTPUT_UNKNOWN;
static int current_battery_mV = 5000; /* Default value of USB supply */

nrfx_wdt_channel_id m_channel_id;

static int nwk_connected = 0;
static bool ota_turbo_poll_active = false;

#define SLEEP_MONITOR_LOG_INTERVAL 128U
#define SENSOR_SCHEDULE_RETRY_MS   1000U
#define OTA_TURBO_POLL_TIMEOUT_MS  120000U
#define OTA_ABORT_RECOVERY_POLL_MS 600000U
#define OTA_POLL_INTERVAL_MS       1000U
#define NORMAL_POLL_INTERVAL_MS    15000U
#define SIGNAL50_RECOVERY_RETRY_MS 1000U
#define SIGNAL50_RECOVERY_MAX_ATTEMPTS 60U
#define SIGNAL50_RECOVERY_INIT_ATTEMPT 3U

APP_TIMER_DEF(m_sensor_schedule_timer);
APP_TIMER_DEF(m_ota_abort_recovery_timer);

static bool sensor_timer_running = false;
static volatile bool ota_abort_recovery_stop_pending = false;
static bool signal50_recovery_active = false;
static uint8_t signal50_recovery_attempts = 0;

static uint32_t zb_delay_to_ms(zb_time_t delay_bi)
{
    uint32_t delay_ms = ZB_TIME_BEACON_INTERVAL_TO_MSEC(delay_bi);

    return delay_ms == 0 ? 1 : delay_ms;
}

static void sensor_schedule_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    sensor_timer_running = false;
    schedule_call_is_pending = true;
}

static void ota_abort_recovery_timer_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);

    ota_abort_recovery_stop_pending = true;
}

/* This early clock initialization is needed for early log timestamps.
Otherwise, ZBOSS takes care of the clock when it starts.
 */
static void clock_init(void)
{
    ret_code_t err_code = nrf_drv_clock_init();
    uint32_t wait_loops = 0;

    if ((err_code == NRF_SUCCESS) || (err_code == NRF_ERROR_MODULE_ALREADY_INITIALIZED))
    {
        nrf_drv_clock_lfclk_request(NULL);

        while (!nrf_drv_clock_lfclk_is_running() && (wait_loops < 100000UL))
        {
            wait_loops++;
            nrf_delay_us(10);
        }

        APP_ERROR_CHECK_BOOL(nrf_drv_clock_lfclk_is_running());
    }
    else
    {
        APP_ERROR_CHECK(err_code);
    }
}

static void sensor_schedule_after_ms(uint32_t delay_ms)
{
    ret_code_t err_code;

    if (sensor_timer_running)
    {
        UNUSED_RETURN_VALUE(app_timer_stop(m_sensor_schedule_timer));
        sensor_timer_running = false;
    }

    err_code = app_timer_start(m_sensor_schedule_timer, APP_TIMER_TICKS(delay_ms), NULL);
    if (err_code == NRF_SUCCESS)
    {
        sensor_timer_running = true;
    }
    else
    {
        NRF_LOG_WARNING("sensor schedule timer start failed: 0x%x", err_code);
    }
}

static void sensor_schedule_after_zb_delay(zb_time_t delay_bi)
{
    sensor_schedule_after_ms(zb_delay_to_ms(delay_bi));
}

static void sleep_monitor_note_return(void)
{
    static uint32_t can_sleep_returns = 0;
    static uint32_t last_log_ticks = 0;
    static bool     last_log_valid = false;

    can_sleep_returns++;

    if ((can_sleep_returns % SLEEP_MONITOR_LOG_INTERVAL) == 0)
    {
        uint32_t now_ticks = app_timer_cnt_get();
        uint32_t elapsed_ticks = last_log_valid ? app_timer_cnt_diff_compute(now_ticks, last_log_ticks) : 0;

        NRF_LOG_INFO("sleep monitor: %lu CAN_SLEEP returns, %lu ticks since last report",
                     can_sleep_returns,
                     elapsed_ticks);

        last_log_ticks = now_ticks;
        last_log_valid = true;
    }
}

static void ota_awake_monitor_note(void)
{
    static uint32_t ota_can_sleep_skips = 0;
    static uint32_t last_log_ticks = 0;
    static bool     last_log_valid = false;

    ota_can_sleep_skips++;

    if ((ota_can_sleep_skips % SLEEP_MONITOR_LOG_INTERVAL) == 0)
    {
        uint32_t now_ticks = app_timer_cnt_get();
        uint32_t elapsed_ticks = last_log_valid ? app_timer_cnt_diff_compute(now_ticks, last_log_ticks) : 0;

        NRF_LOG_INFO("OTA awake monitor: %lu CAN_SLEEP skips, %lu ticks since last report",
                     ota_can_sleep_skips,
                     elapsed_ticks);

        last_log_ticks = now_ticks;
        last_log_valid = true;
    }
}


static void log_float_as_binary(const zb_float32_t *f)
{
#if (NRF_LOG_ENABLED && (NRF_LOG_LEVEL >= NRF_LOG_SEVERITY_DEBUG))
    static char dbg_bin_f[33] = { 0 };

    uint32_t p = 0x80000000;
    unsigned i;
    for (i = 0; i < 32; i++) {
        dbg_bin_f[i] = (f->v & p) ? '1' : '0';
        p = p >> 1;
    }
    NRF_LOG_DEBUG("Bin: %s", dbg_bin_f);
    NRF_LOG_FLUSH();
#endif
}


void led_on(void)
{
    nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0, 15));
}

void led_off(void)
{
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 15));
}

void led_toggle(void)
{
    nrf_gpio_pin_toggle(NRF_GPIO_PIN_MAP(0, 15));
}

static void reboot(zb_uint8_t param)
{
    UNUSED_PARAMETER(param);
    NRF_LOG_FINAL_FLUSH();
    // To allow the buffer to be flushed by the host.
    nrf_delay_ms(100);
    zb_osif_abort();
}

static void dfu_finalize_callback(zb_uint8_t param)
{
    UNUSED_PARAMETER(param);
    app_dfu_finalize_and_reset();
}

static void signal50_recover_network_steering(zb_uint8_t param)
{
    zb_bool_t started;
    zb_bool_t init_started = ZB_FALSE;
    zb_ret_t  sched_ret;

    NRF_LOG_WARNING("Signal 50 recovery: attempt=%u status=%u connected=%d",
                    signal50_recovery_attempts + 1,
                    param,
                    nwk_connected);
    if (nwk_connected)
    {
        signal50_recovery_active = false;
        signal50_recovery_attempts = 0;
        return;
    }

    signal50_recovery_attempts++;

    if (signal50_recovery_attempts % 10 == 2) {
        NRF_LOG_WARNING("Signal 50 recovery: calling zboss_start_continue()");
        zboss_start_continue();
    }

    if (signal50_recovery_attempts >= SIGNAL50_RECOVERY_INIT_ATTEMPT)
    {
        init_started = bdb_start_top_level_commissioning(ZB_BDB_INITIALIZATION);
    }

    started = bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
    NRF_LOG_WARNING("Signal 50 recovery: init_started=%u steering_started=%u",
                    init_started ? 1 : 0,
                    started ? 1 : 0);

    if (started)
    {
        signal50_recovery_active = false;
        signal50_recovery_attempts = 0;
        return;
    }

    if (signal50_recovery_attempts >= SIGNAL50_RECOVERY_MAX_ATTEMPTS)
    {
        NRF_LOG_ERROR("Signal 50 recovery: giving up after %u attempts", signal50_recovery_attempts);
        signal50_recovery_active = false;
        signal50_recovery_attempts = 0;
        return;
    }

    sched_ret = ZB_SCHEDULE_APP_ALARM(signal50_recover_network_steering,
                                      param,
                                      ZB_MILLISECONDS_TO_BEACON_INTERVAL(SIGNAL50_RECOVERY_RETRY_MS));
    if (sched_ret != RET_OK)
    {
        NRF_LOG_WARNING("Signal 50 recovery reschedule failed: %hd", sched_ret);
        signal50_recovery_active = false;
        signal50_recovery_attempts = 0;
    }
}

static const char * ota_status_name(zb_uint8_t status)
{
    switch (status)
    {
        case ZB_ZCL_OTA_UPGRADE_STATUS_START:
            return "START";
        case ZB_ZCL_OTA_UPGRADE_STATUS_APPLY:
            return "APPLY";
        case ZB_ZCL_OTA_UPGRADE_STATUS_RECEIVE:
            return "RECEIVE";
        case ZB_ZCL_OTA_UPGRADE_STATUS_FINISH:
            return "FINISH";
        case ZB_ZCL_OTA_UPGRADE_STATUS_ABORT:
            return "ABORT";
        case ZB_ZCL_OTA_UPGRADE_STATUS_CHECK:
            return "CHECK";
        case ZB_ZCL_OTA_UPGRADE_STATUS_OK:
            return "OK";
        case ZB_ZCL_OTA_UPGRADE_STATUS_ERROR:
            return "ERROR";
        case ZB_ZCL_OTA_UPGRADE_STATUS_REQUIRE_MORE_IMAGE:
            return "REQUIRE_MORE_IMAGE";
        case ZB_ZCL_OTA_UPGRADE_STATUS_BUSY:
            return "BUSY";
        case ZB_ZCL_OTA_UPGRADE_STATUS_SERVER_NOT_FOUND:
            return "SERVER_NOT_FOUND";
        default:
            return "?";
    }
}

static void ota_turbo_poll_start(bool log_change)
{
    if (!ota_turbo_poll_active)
    {
        ota_turbo_poll_active = true;
        zb_zdo_pim_permit_turbo_poll(ZB_TRUE);
        zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(OTA_POLL_INTERVAL_MS));
        zb_zdo_pim_set_long_poll_interval(OTA_POLL_INTERVAL_MS);
        NRF_LOG_INFO("OTA: turbo poll start timeout=%u ms poll=%u ms",
                     OTA_TURBO_POLL_TIMEOUT_MS,
                     OTA_POLL_INTERVAL_MS);
    }
    else if (log_change)
    {
        NRF_LOG_INFO("OTA: turbo poll refresh timeout=%u ms", OTA_TURBO_POLL_TIMEOUT_MS);
    }

    zb_zdo_pim_start_turbo_poll_continuous(OTA_TURBO_POLL_TIMEOUT_MS);
}

static void ota_turbo_poll_stop(void)
{
    if (ota_turbo_poll_active)
    {
        ota_turbo_poll_active = false;
        NRF_LOG_INFO("OTA: turbo poll stop");
        zb_zdo_pim_turbo_poll_continuous_leave(0);
        zb_zdo_pim_permit_turbo_poll(ZB_FALSE);
        zb_zdo_pim_set_long_poll_interval(NORMAL_POLL_INTERVAL_MS);
        zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(NORMAL_POLL_INTERVAL_MS));
    }
}

static void ota_abort_recovery_poll_start(void)
{
    ret_code_t err_code;

    ota_turbo_poll_start(true);

    err_code = app_timer_stop(m_ota_abort_recovery_timer);
    if ((err_code != NRF_SUCCESS) && (err_code != NRF_ERROR_INVALID_STATE))
    {
        NRF_LOG_WARNING("OTA abort recovery timer stop failed: 0x%x", err_code);
    }

    ota_abort_recovery_stop_pending = false;

    err_code = app_timer_start(m_ota_abort_recovery_timer,
                               APP_TIMER_TICKS(OTA_ABORT_RECOVERY_POLL_MS),
                               NULL);
    if (err_code == NRF_SUCCESS)
    {
        NRF_LOG_INFO("OTA abort recovery: fast poll for %u ms", OTA_ABORT_RECOVERY_POLL_MS);
    }
    else
    {
        NRF_LOG_WARNING("OTA abort recovery timer start failed: 0x%x", err_code);
    }
}

static void ota_abort_recovery_check(void)
{
    if (ota_abort_recovery_stop_pending)
    {
        ota_abort_recovery_stop_pending = false;
        NRF_LOG_INFO("OTA abort recovery: restore normal poll");
        ota_turbo_poll_stop();
    }
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
        wait_interval = ZB_MILLISECONDS_TO_BEACON_INTERVAL(60000);
    }
    sensor_schedule_after_zb_delay(wait_interval);
}


/**@brief Check if a call to sensor_loop_helper() needs to be scheduled and attempt to schedule it.
 *
 */
bool check_pending_sensor_schedule_call(void)
{
    if (schedule_call_is_pending) {
        zb_ret_t zb_err_code = ZB_SCHEDULE_APP_CALLBACK(sensor_loop_helper, 0);
        if (zb_err_code == RET_OK) {
            schedule_call_is_pending = false;
            return true;
        } else if (zb_err_code == RET_OVERFLOW) {
            NRF_LOG_WARNING("Can not schedule sensor callback, queue is full.");
//            ZB_ERROR_CHECK(zb_err_code);
            schedule_call_is_pending = false;
            sensor_schedule_after_ms(SENSOR_SCHEDULE_RETRY_MS);
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
                        sensor_request_calibration(i_value);
                    }
                    if (ep_id == SECOND_ENDPOINT) {
                        sensor_set_calibration_target(i_value);
                    }
                }

            } else {
                p_device_cb_param->status = RET_NOT_IMPLEMENTED;
                NRF_LOG_INFO("Unhandled cluster attribute id: %d 0x%x", cluster_id, attr_id);
            }
            break;
        case ZB_ZCL_OTA_UPGRADE_VALUE_CB_ID:
        {
            zb_zcl_ota_upgrade_value_param_t * p_ota_upgrade_value = &(p_device_cb_param->cb_param.ota_value_param);
            zb_uint8_t ota_status_in = p_ota_upgrade_value->upgrade_status;

            if (ota_status_in != ZB_ZCL_OTA_UPGRADE_STATUS_RECEIVE)
            {
                NRF_LOG_INFO("OTA cb: in=%s(%u) ep=%u",
                             (uint32_t)ota_status_name(ota_status_in),
                             ota_status_in,
                             p_device_cb_param->endpoint);
            }

            switch (p_ota_upgrade_value->upgrade_status)
            {
                case ZB_ZCL_OTA_UPGRADE_STATUS_START:
                    /* Check if OTA client is in the middle of image download.
                        If so, silently ignore the second QueryNextImageResponse packet from OTA server. */
                    if (zb_zcl_ota_upgrade_get_ota_status(p_device_cb_param->endpoint) != ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL)
                    {
                        p_ota_upgrade_value->upgrade_status = ZB_ZCL_OTA_UPGRADE_STATUS_BUSY;
                    }

                    /* Check if we're not downgrading.
                       If we do, let's politely say no since we do not support that. */
                    else if (p_ota_upgrade_value->upgrade.start.file_version > ota_attr.file_version)
                    {
                        ota_turbo_poll_start(true);
                        p_ota_upgrade_value->upgrade_status = ZB_ZCL_OTA_UPGRADE_STATUS_OK;
                    }
                    else
                    {
                        p_ota_upgrade_value->upgrade_status = ZB_ZCL_OTA_UPGRADE_STATUS_ABORT;
                    }
                    break;

                case ZB_ZCL_OTA_UPGRADE_STATUS_RECEIVE:
                    /* Process image block. */
                    ota_turbo_poll_start(false);
                    p_ota_upgrade_value->upgrade_status = zb_process_chunk(p_ota_upgrade_value, bufid);
                    led_toggle();
                    break;

                case ZB_ZCL_OTA_UPGRADE_STATUS_CHECK:
                    p_ota_upgrade_value->upgrade_status = ZB_ZCL_OTA_UPGRADE_STATUS_OK;
                    break;

                case ZB_ZCL_OTA_UPGRADE_STATUS_APPLY:
                    led_on();
                    p_ota_upgrade_value->upgrade_status = ZB_ZCL_OTA_UPGRADE_STATUS_OK;
                    break;

                case ZB_ZCL_OTA_UPGRADE_STATUS_FINISH:
                {
                    /* It is time to upgrade FW. */
                    bool finalize_ready = app_dfu_finalize_is_ready();
                    if (finalize_ready)
                    {
                        p_ota_upgrade_value->upgrade_status = ZB_ZCL_OTA_UPGRADE_STATUS_OK;
                        if (ZB_SCHEDULE_APP_CALLBACK(dfu_finalize_callback, 0) != RET_OK)
                        {
                            NRF_LOG_ERROR("OTA finish: unable to schedule DFU finalization");
                            ota_turbo_poll_stop();
                            p_ota_upgrade_value->upgrade_status = ZB_ZCL_OTA_UPGRADE_STATUS_ABORT;
                        }
                    }
                    else
                    {
                        NRF_LOG_ERROR("OTA finish: no finalized DFU image is ready");
                        ota_turbo_poll_stop();
                        p_ota_upgrade_value->upgrade_status = ZB_ZCL_OTA_UPGRADE_STATUS_ABORT;
                    }
                    break;
                }

                case ZB_ZCL_OTA_UPGRADE_STATUS_ABORT:
                    NRF_LOG_INFO("Zigbee DFU Aborted");
                    p_ota_upgrade_value->upgrade_status = ZB_ZCL_OTA_UPGRADE_STATUS_ABORT;
                    led_off();
                    ota_abort_recovery_poll_start();
                    app_zb_abort_dfu_preserve_progress();
                    break;
                default:
                    NRF_LOG_WARNING("OTA cb: unhandled status=%u", p_ota_upgrade_value->upgrade_status);
                    p_device_cb_param->status = RET_NOT_IMPLEMENTED;
                    break;
            }

            if ((ota_status_in != ZB_ZCL_OTA_UPGRADE_STATUS_RECEIVE) ||
                (p_ota_upgrade_value->upgrade_status != ZB_ZCL_OTA_UPGRADE_STATUS_BUSY))
            {
                NRF_LOG_INFO("OTA cb: out=%s(%u) cb_status=%hd",
                             (uint32_t)ota_status_name(p_ota_upgrade_value->upgrade_status),
                             p_ota_upgrade_value->upgrade_status,
                             p_device_cb_param->status);
            }
            break;
        }
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
    zb_ret_t                       status = ZB_GET_APP_SIGNAL_STATUS(bufid);
//    zb_ret_t                       zb_err_code;

    if (sig != ZB_COMMON_SIGNAL_CAN_SLEEP)
    {
        NRF_LOG_INFO("zboss signal %hu", sig);
        NRF_LOG_FLUSH();
    }

    switch(sig)
    {
        case ZB_NLME_STATUS_INDICATION:

            zb_zdo_signal_nlme_status_indication_params_t
                *nlme_status_ind = ZB_ZDO_SIGNAL_GET_PARAMS(p_sg_p,
                     zb_zdo_signal_nlme_status_indication_params_t);

            if (nlme_status_ind) {
                NRF_LOG_WARNING("NLME_STATUS_INDICATION: status=%hhd addr=%hd cmd=%hhd",
                    nlme_status_ind->nlme_status.status,
                    nlme_status_ind->nlme_status.network_addr,
                    nlme_status_ind->nlme_status.unknown_command_id);
            } else {
                NRF_LOG_WARNING("NLME_STATUS_INDICATION: no data");
            }
            NRF_LOG_FLUSH();

//            if (nlme_status_ind->nlme_status.status ==  ZB_NWK_COMMAND_STATUS_BAD_KEY_SEQUENCE_NUMBER) {
                                // optional check connection
                                // optional rejoin if necessary
//            }

            /* Signal 50 is not handled by this SDK's zigbee_default_signal_handler().
             * Newer ZBOSS headers identify this area of the signal range differently,
             * so consume it here without interpreting the payload.
             */
//            NRF_LOG_WARNING("Consumed ZBOSS signal 50, status=%hd", status);
            if (!nwk_connected && !signal50_recovery_active)
            {
                zb_ret_t sched_ret;

                signal50_recovery_active = true;
                signal50_recovery_attempts = 0;
                sched_ret = ZB_SCHEDULE_APP_ALARM(signal50_recover_network_steering,
                                                  (zb_uint8_t)status,
                                                  ZB_MILLISECONDS_TO_BEACON_INTERVAL(SIGNAL50_RECOVERY_RETRY_MS));
                if (sched_ret != RET_OK)
                {
                    NRF_LOG_WARNING("Signal 50 recovery schedule failed: %hd", sched_ret);
                    signal50_recovery_active = false;
                }
            }
            break;
        case ZB_BDB_SIGNAL_DEVICE_REBOOT:
            /* fall-through */
        case ZB_BDB_SIGNAL_STEERING:
            /* Call default signal handler. */
            ZB_ERROR_CHECK(zigbee_default_signal_handler(bufid));

            if (RET_OK == status) {
                signal50_recovery_active = false;
                signal50_recovery_attempts = 0;
                UNUSED_RETURN_VALUE(ZB_SCHEDULE_APP_ALARM_CANCEL(signal50_recover_network_steering,
                                                                  ZB_ALARM_ANY_PARAM));
                sensor_loop_helper(0);
                check_pending_sensor_schedule_call();
                nwk_connected = 1;
            }
            break;

        case ZB_ZDO_SIGNAL_LEAVE:
            nwk_connected = 0;
            break;

        case ZB_COMMON_SIGNAL_CAN_SLEEP:
            /* Zigbee stack can enter sleep state.
             * If the application wants to proceed, it should call zb_sleep_now() function.
             *
             * Note: if the application shares some resources between Zigbee stack and other tasks/contexts,
             *       device disabling should be overwritten by implementing one of the weak functions inside zb_nrf52840_common.c.
            */
            check_pending_sensor_schedule_call();
            if (ota_turbo_poll_active)
            {
//                ota_awake_monitor_note();
            }
            else
            {
                zb_sleep_now();
                sleep_monitor_note_return();
            }
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

    err_code = app_timer_create(&m_sensor_schedule_timer,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                sensor_schedule_timer_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&m_ota_abort_recovery_timer,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                ota_abort_recovery_timer_handler);
    APP_ERROR_CHECK(err_code);

}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(app_timer_cnt_get);
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
    nrfx_wdt_channel_feed(m_channel_id);
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


    if (!nwk_connected)
        return;

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


    int v10 = mv / 100;
    zb_zcl_set_attr_val(FIRST_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
                                     ZB_ZCL_CLUSTER_SERVER_ROLE,
                                     ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_VOLTAGE_ID,
                                     (zb_uint8_t *)&v10,
                                     ZB_FALSE);

    int percentage = voltage_to_soc(mv);
    zb_zcl_set_attr_val(FIRST_ENDPOINT,
                                     ZB_ZCL_CLUSTER_ID_POWER_CONFIG,
                                     ZB_ZCL_CLUSTER_SERVER_ROLE,
                                     ZB_ZCL_ATTR_POWER_CONFIG_BATTERY_PERCENTAGE_REMAINING_ID,
                                     (zb_uint8_t *)&percentage,
                                     ZB_FALSE);

}

void log_chip_info(void)
{
    uint32_t rr = nrf_power_resetreas_get();
    nrf_power_resetreas_clear(rr);
    NRF_LOG_INFO("reset reason = 0x%x", rr);
    NRF_LOG_FLUSH();
    uint32_t part = NRF_FICR->INFO.PART;
    NRF_LOG_INFO("chip part no = 0x%x", part);
    uint32_t var = NRF_FICR->INFO.VARIANT;
    NRF_LOG_INFO("chip variant = %c%c%c%c", var&0xff, (var>>8)&0xff, (var>>16)&0xff, (var>>24)&0xff);
    uint32_t pack = NRF_FICR->INFO.PACKAGE;
    NRF_LOG_INFO("chip package = 0x%x", pack);
}

void led_blink(int count)
{
    int i = 0;

    for (i = 0; i < count; i++) {
        nrf_delay_ms(250);
        led_on();
        nrf_delay_ms(250);
        led_off();
    }

}

/**@brief Function for application main entry.
 */
int main(void)
{
    zb_ret_t       zb_err_code;
    zb_ieee_addr_t ieee_addr;

    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 15));

    /* Initialize timer, logging system and GPIOs. */
    clock_init();
    timers_init();
    log_init();
    log_chip_info();

    /* Indicate boot progress */
    led_blink(1);

    sensor_init (zb_attr_update_from_scd40);

    calibration_test();
    zb_trans_set_tx_power(-8);

    /* Set Zigbee stack logging level and traffic dump subsystem. */
    ZB_SET_TRACE_LEVEL(ZIGBEE_TRACE_LEVEL);
    ZB_SET_TRACE_MASK(ZIGBEE_TRACE_MASK);
    ZB_SET_TRAF_DUMP_OFF();

    /* Indicate boot progress */
    led_blink(1);

    /* Initialize the Zigbee DFU Transport */
    zb_dfu_init(OTA_ENDPOINT);

    led_blink(1);

    /* Initialize Zigbee stack. */
    ZB_INIT("air_sensor");

    /* Indicate successful initialization of Zigbee subsystem */
    led_blink(1);

    /* Set device address to the value read from FICR registers. */
    zb_osif_get_ieee_eui64(ieee_addr);
    zb_set_long_address(ieee_addr);

    zb_set_network_ed_role(IEEE_CHANNEL_MASK);
    zigbee_erase_persistent_storage(ERASE_PERSISTENT_CONFIG);

    zb_set_ed_timeout(ED_AGING_TIMEOUT_64MIN);
    zb_set_keepalive_timeout(ZB_MILLISECONDS_TO_BEACON_INTERVAL(NORMAL_POLL_INTERVAL_MS));
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
        app_sched_execute();
        check_pending_sensor_schedule_call();
        ota_abort_recovery_check();
        nrfx_wdt_channel_feed(m_channel_id);
        UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
    }
}


/**
 * @}
 */
