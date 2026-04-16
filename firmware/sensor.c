#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrfx_twi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "zb_error_handler.h"

#include "sensor.h"

/*
    Global settings
*/
#define VCC_POWER_CONTROL_PIN NRF_GPIO_PIN_MAP(0, 13)

#define SENSOR_INTERVAL 300000  /* interval between measurement series; ms */
#define DATA_INTERVAL 5000  /* interval between periodic measurements: 5000 for normal, 30000 for low power; ms */
#define STATUS_INTERVAL (DATA_INTERVAL/2+100)  /* interval between data status checks; ms */
#define NUM_PRE_READINGS   3 /* Number of warmup readings, which are discarded  */
#define NUM_READINGS   1 /* Number of readings to collect for averaging */

#define TEMPERATURE_CORRECTION 120 /* Additional offset of the temperature sensor, x100 C */

/* I2C definitions and code.
   I2C is called TWI in Nordic's terminology.
 */

#define SCD40_ADDR          (0x62U >> 0)

/*
#define SCD40_START_PERIODIC_MEASUREMENT     0x21b1
#define SCD40_READ_MEASUREMENT               0xec05
#define SCD40_STOP_PERIODIC_MEASUREMENT      0x3f86
#define SCD40_GET_DATA_READY_STATUS          0xe4b8
*/

#define SCD40_START_PERIODIC_MEASUREMENT     {0x21, 0xb1}
#define SCD40_START_PERIODIC_MEASUREMENT_LP  {0x21, 0xac}
#define SCD40_READ_MEASUREMENT               {0xec, 0x05}
#define SCD40_STOP_PERIODIC_MEASUREMENT      {0x3f, 0x86}
#define SCD40_GET_DATA_READY_STATUS          {0xe4, 0xb8}


/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

#define SCL_PIN NRF_GPIO_PIN_MAP(1, 6)
#define SDA_PIN NRF_GPIO_PIN_MAP(1, 4)

/* Indicates if operation on TWI has ended. */
static volatile bool scd40_measurement_started = false;
static volatile bool m_xfer_done = false;
static volatile bool m_xfer_success = false;
static volatile int m_xfer_evt_code = NRFX_TWI_EVT_DONE;

/* TWI instance. */
static const nrfx_twi_t m_twi = NRFX_TWI_INSTANCE(TWI_INSTANCE_ID);

/* Buffer for samples read from CO2 sensor. */
static uint8_t m_ready[3];

/* Forward declaration for the scheduler */
void sensor_loop_helper(zb_uint8_t position);
void twi_uninit (void);
void twi_init (void);

/* Sensor data */
static struct {
/* Raw data as read from the sensor */
    uint8_t raw[9];

/* Accumulated data */
    uint16_t count;
    uint32_t accum_CO2;
    uint32_t accum_temperature;
    uint32_t accum_humidity;

/* Final converted sensor output data */
    sensor_output_t out;
} scd40_measurement_data;

/* Callback to update  */
static data_cb_t m_data_cb = NULL;


/* I2C transfer descriptors */
static uint8_t scd40_start_measurement_cmd[] = SCD40_START_PERIODIC_MEASUREMENT;
//static uint8_t scd40_start_measurement_cmd[] = SCD40_START_PERIODIC_MEASUREMENT_LP;
static uint8_t scd40_read_data_cmd[]         = SCD40_READ_MEASUREMENT;
static uint8_t scd40_stop_measurement_cmd[]  = SCD40_STOP_PERIODIC_MEASUREMENT;
static uint8_t scd40_data_ready_cmd[]        = SCD40_GET_DATA_READY_STATUS;


nrfx_twi_xfer_desc_t scd40_start_measurement_desc =
    NRFX_TWI_XFER_DESC_TX(SCD40_ADDR, scd40_start_measurement_cmd, sizeof(scd40_start_measurement_cmd));

nrfx_twi_xfer_desc_t scd40_read_data_desc =
    NRFX_TWI_XFER_DESC_TXRX(SCD40_ADDR, scd40_read_data_cmd, sizeof(scd40_read_data_cmd), scd40_measurement_data.raw, sizeof(scd40_measurement_data.raw));

nrfx_twi_xfer_desc_t scd40_stop_measurement_desc =
    NRFX_TWI_XFER_DESC_TX(SCD40_ADDR, scd40_stop_measurement_cmd, sizeof(scd40_stop_measurement_cmd));

nrfx_twi_xfer_desc_t scd40_data_ready_desc =
    NRFX_TWI_XFER_DESC_TXRX(SCD40_ADDR, scd40_data_ready_cmd, sizeof(scd40_data_ready_cmd), m_ready, sizeof(m_ready));

static uint8_t test_xfer_data[] = { 0xff, 0xff };
nrfx_twi_xfer_desc_t test_transfer_desc =
    NRFX_TWI_XFER_DESC_TX(SCD40_ADDR+2, test_xfer_data, sizeof(test_xfer_data));

/* Possible loop action outcomes */
typedef enum sensor_loop_action_result {
    SLA_SUCCESS,
    SLA_RETRY,
    SLA_FAIL,
} sensor_loop_action_result_t;

/**
 * @brief Command to start measurements.
   Non-blocking, returns immediately.
 */
sensor_loop_action_result_t scd40_start_measurement(void)
{
//    while (nrfx_twi_is_busy(&m_twi))
//        nrf_delay_ms(1);

    ret_code_t err_code;

    err_code = nrfx_twi_xfer(&m_twi, &scd40_start_measurement_desc, 0);

    NRF_LOG_DEBUG("Start: %d", err_code);

    if (NRF_SUCCESS != err_code) {
        m_xfer_done = false;
        m_xfer_success = false;
        return SLA_FAIL;
    }

    nrf_delay_ms(10);
    return SLA_SUCCESS;
}

/**
 * @brief Command to check data status.
   Returns true if data is ready.
 */
sensor_loop_action_result_t scd40_is_data_ready(void)
{
    if (!m_xfer_done) {
        return SLA_RETRY;
    }

    if (!scd40_measurement_started) {
        return scd40_start_measurement();
    }

//    while (nrfx_twi_is_busy(&m_twi))
//        nrf_delay_ms(1);

    ret_code_t err_code;

    /* Send request */
    err_code = nrfx_twi_xfer(&m_twi, &scd40_data_ready_desc, 0);

    if (NRF_SUCCESS != err_code) {
        m_xfer_done = true;
        return SLA_FAIL;
    }

    /* Wait until ready */
    while (!m_xfer_done) {
        nrf_delay_ms(1);
    }

    nrf_delay_ms(10);

    if (!m_xfer_success)
        return SLA_FAIL;

    uint16_t status = uint16_big_decode(m_ready);

    /* Check the least significant 11 bits for zero */
    if ((status & 0x3ff) == 0)
    /* Not ready */
        return SLA_RETRY;

    return SLA_SUCCESS;
}

/**
 * @brief Command to reset measurement data to 0.
   Always returns true.
 */
sensor_loop_action_result_t scd40_reset_data(void)
{
    scd40_measurement_data.accum_CO2 = 0;
    scd40_measurement_data.accum_temperature = 0;
    scd40_measurement_data.accum_humidity = 0;
    scd40_measurement_data.count = 0;

    return SLA_SUCCESS;
}

/**
 * @brief Command to read measurement data.
   Always returns true.
 */
sensor_loop_action_result_t scd40_read_data(int count)
{
    if (!m_xfer_done) {
        return SLA_RETRY;
    }

    sensor_loop_action_result_t data_ready = scd40_is_data_ready();

    if (data_ready != SLA_SUCCESS)
        return SLA_FAIL;

//    while (nrfx_twi_is_busy(&m_twi))
//        nrf_delay_ms(1);

    ret_code_t err_code;

    /* Send request */
    err_code = nrfx_twi_xfer(&m_twi, &scd40_read_data_desc, 0);

    if (NRF_SUCCESS != err_code) {
        m_xfer_done = true;
        return SLA_FAIL;
    }

    /* Wait until ready */
    while (!m_xfer_done) {
        nrf_delay_ms(1);
    }

    nrf_delay_ms(10);

    if (!m_xfer_success)
        return SLA_FAIL;


//    while (nrfx_twi_is_busy(&m_twi))
//        nrf_delay_ms(1);

    /* CO2 is reported in ppm, can be accumulated directly */
    uint32_t ppm_CO2 = uint16_big_decode(&scd40_measurement_data.raw[0]);
    scd40_measurement_data.accum_CO2 += ppm_CO2;

    /* Temperature conversion and accumulation */
    uint32_t c_temp = uint16_big_decode(&scd40_measurement_data.raw[3]);
//    c_temp = (c_temp * 175) / (1 << 16) - 45;
    scd40_measurement_data.accum_temperature += c_temp;

    /* Humidity conversion and accumulation */
    uint32_t c_hum = uint16_big_decode(&scd40_measurement_data.raw[6]);
//    c_hum = (c_hum * 100) / (1 << 16);
    scd40_measurement_data.accum_humidity += c_hum;

    scd40_measurement_data.count += 1;

    NRF_LOG_INFO("read %u %u %u", ppm_CO2, (175*c_temp)>>16, (100*c_hum)>>16);

//    if (scd40_measurement_data.count < NUM_READINGS)
    if (scd40_measurement_data.count < count)
        return SLA_RETRY;

    return SLA_SUCCESS;
}

sensor_loop_action_result_t scd40_read_data1(void)
{
    return scd40_read_data(NUM_PRE_READINGS);
}

sensor_loop_action_result_t scd40_read_dataN(void)
{
    return scd40_read_data(NUM_READINGS);
}

/**
 * @brief Command to stop.
 */
sensor_loop_action_result_t scd40_stop_measurement(void)
{
//    while (nrfx_twi_is_busy(&m_twi))
//        nrf_delay_ms(1);

    ret_code_t err_code;

    err_code = nrfx_twi_xfer(&m_twi, &scd40_stop_measurement_desc, 0);
    NRF_LOG_DEBUG("Stop: %d", err_code);
#if 0
    if (NRF_SUCCESS != err_code) {
        m_xfer_done = true;
        m_xfer_success = false;
        return SLA_FAIL;
    }
#endif
    return SLA_SUCCESS;
}

/**
 * @brief Power up the sensor
 */
sensor_loop_action_result_t scd40_power_on(void)
{
    nrf_gpio_pin_set(VCC_POWER_CONTROL_PIN);
    twi_init();
    return SLA_SUCCESS;
}


/**
 * @brief Kill sensor power to save battery energy
 */
sensor_loop_action_result_t scd40_power_off(void)
{
    twi_uninit();
    nrf_gpio_pin_clear(VCC_POWER_CONTROL_PIN);
    return SLA_SUCCESS;
}

/**
 * @brief Update Zigbee attributes with last measurement data
 */
sensor_loop_action_result_t scd40_update_zb_data(void)
{
    if (scd40_measurement_data.count < 1)
        return SLA_SUCCESS;

    scd40_measurement_data.out.ppm_CO2 =
        scd40_measurement_data.accum_CO2 / scd40_measurement_data.count;

/* Temperature * 100, C */
    scd40_measurement_data.out.c_temperature =
        (scd40_measurement_data.accum_temperature * 175 * (100/4)) / (scd40_measurement_data.count << (16 - 2)) - (4500 - TEMPERATURE_CORRECTION);

/* Humidity * 100, % */
    scd40_measurement_data.out.p_humidity =
        (scd40_measurement_data.accum_humidity * (10000/16)) / (scd40_measurement_data.count << (16 - 4));

    if (m_data_cb)
        m_data_cb(&scd40_measurement_data.out);

    return SLA_SUCCESS;
}

/**
 * @brief TWI events handler.
 */
void twi_handler(nrfx_twi_evt_t const * p_event, void * p_context)
{
    m_xfer_done = true;
    m_xfer_evt_code = p_event->type;
    switch (p_event->type)
    {
        case NRFX_TWI_EVT_DONE:
            m_xfer_success = true;

            if (p_event->xfer_desc.type == NRFX_TWI_XFER_TX)
            {
                if (p_event->xfer_desc.p_primary_buf == &scd40_start_measurement_cmd[0])
                    scd40_measurement_started = true;
                if (p_event->xfer_desc.p_primary_buf == &scd40_stop_measurement_cmd[0])
                    scd40_measurement_started = false;
            }
            break;
        default:
            m_xfer_success = false;
            break;
    }
}

/**
 * @brief I2C initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;

    const nrfx_twi_config_t twi_scd40_config = {
       .scl                = SCL_PIN,
       .sda                = SDA_PIN,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .hold_bus_uninit = 0,
    };

    do {
        err_code = nrfx_twi_init(&m_twi, &twi_scd40_config, twi_handler, NULL);
        if (NRF_SUCCESS != err_code) {
            nrf_delay_ms(100);
        }
    } while (NRF_SUCCESS != err_code);

    NRF_LOG_DEBUG("TWI Init successful.");
    nrfx_twi_enable(&m_twi);
}

void twi_uninit (void)
{
    nrfx_twi_disable(&m_twi);
    nrfx_twi_uninit(&m_twi);
}

typedef sensor_loop_action_result_t (*sensor_loop_function_t)(void);

static uint8_t loop_position = 0;
/* Number of retries after action call has returned false */
static uint8_t loop_fail_count = 0;
/* Maximum number of retries before resetting */
static const uint8_t max_fail_count = 20;

#define USE_STOP 1

struct {
    zb_time_t interval; /* Time to wait before function call */
    sensor_loop_function_t action;
} sensor_loop_tbl[] = {
    {
        0,
        scd40_stop_measurement,
    },
#if USE_STOP
    {
        ZB_MILLISECONDS_TO_BEACON_INTERVAL(500), /* Stop measurements command execution time is 500ms */
        scd40_power_off,
    },
    {
        ZB_MILLISECONDS_TO_BEACON_INTERVAL(SENSOR_INTERVAL),
        scd40_power_on,
    },
#endif
    {
        ZB_MILLISECONDS_TO_BEACON_INTERVAL(1000), /* Sensor needs 1000ms to enter the idle state */
        scd40_start_measurement,
    },
    {
        ZB_MILLISECONDS_TO_BEACON_INTERVAL(DATA_INTERVAL),
        scd40_reset_data,
    },

#if 1
/* Read and accumulate several measurements */
    {
        ZB_MILLISECONDS_TO_BEACON_INTERVAL(STATUS_INTERVAL),
        scd40_read_data1,
    },

    {
        0,
        scd40_stop_measurement,
    },

    {
        ZB_MILLISECONDS_TO_BEACON_INTERVAL(15000),
        scd40_reset_data,
    },

#endif
/* Read and accumulate several measurements */
    {
        ZB_MILLISECONDS_TO_BEACON_INTERVAL(STATUS_INTERVAL),
        scd40_read_dataN,
    },


    {
        0,
        scd40_update_zb_data,
    },
};

void sensor_init(data_cb_t data_cb)
{
    twi_init();
    nrf_gpio_cfg_output(VCC_POWER_CONTROL_PIN);
    nrf_gpio_pin_set(VCC_POWER_CONTROL_PIN);
    m_data_cb = data_cb;

#if !USE_STOP
//    scd40_start_measurement();
//    nrf_delay_ms(1000);
#endif
}


void test_twi_xfer(void)
{
    nrfx_twi_xfer(&m_twi, &test_transfer_desc, 0);
}

#if 0
void sensor_loop_helper(zb_uint8_t position)
{
    if (position >= ARRAY_SIZE(sensor_loop_tbl))
        position = 0;

    NRF_LOG_DEBUG("Sensor loop: %d", position);

/* Call the current action routine */
    bool result = sensor_loop_tbl[position].action();
/* If successfull - advance the loop*/
    if (result) position += 1;

    if (position >= ARRAY_SIZE(sensor_loop_tbl))
        position = 0;
/* Set the required delay till next call */
    zb_time_t interval = sensor_loop_tbl[position].interval;
/* Schedule next step */
    ZB_ERROR_CHECK(ZB_SCHEDULE_APP_ALARM(sensor_loop_helper, position, interval));
}
#endif

void sensor_loop(zb_time_t *wait_interval, uint8_t *is_idle)
{
    if (loop_position >= ARRAY_SIZE(sensor_loop_tbl))
        loop_position = 0;

    NRF_LOG_DEBUG("Sensor loop: %d", loop_position);

/* Call the current action routine */
    sensor_loop_action_result_t result = sensor_loop_tbl[loop_position].action();

/* If successfull - advance the loop*/
    if (SLA_SUCCESS == result) {

    /* Notify the caller that sensor is being shut down */
#if USE_STOP
        *is_idle = (sensor_loop_tbl[loop_position].action == scd40_power_off);
#else
        *is_idle = (sensor_loop_tbl[loop_position].action == scd40_stop_measurement);
#endif
        loop_position += 1;
        loop_fail_count = 0;
    } else if (SLA_FAIL == result) {
        loop_fail_count += 1;
/* Too many failures - skip the loop to the beginning, stop and reset the sensor*/
        if (loop_fail_count >= max_fail_count) {
            loop_position = 0;
            loop_fail_count = 0;
        }
    }

    if (loop_position >= ARRAY_SIZE(sensor_loop_tbl)) {
        loop_position = 0;
        loop_fail_count = 0;
    }
/* Set the required delay till next call */
    *wait_interval = sensor_loop_tbl[loop_position].interval;


/* Schedule next step */
//    ZB_ERROR_CHECK(ZB_SCHEDULE_APP_ALARM(sensor_loop_helper, loop_position, interval));


}

