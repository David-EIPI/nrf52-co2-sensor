/**
 * Implementation of ZCL concentration cluster for ZBOSS Zigbee stack in Nordic's nRF5 SDK.
 * David Shirvanyants
 * 2026/03
 */
#include "zboss_api.h"
#include "zb_zcl_concentration.h"
#include "app_error.h"
#include "nrf_log.h"

/**@brief Function which pre-validates the value of attributes before they are written.
 * Currently not implemented to avoid floating point arithmetics.
 * 
 * @param [in] attr_id  Attribute ID
 * @param [in] endpoint Endpoint
 * @param [in] p_value  Pointer to the value of the attribute which is to be validated
 * 
 * @return ZB_TRUE if the value is valid, ZB_FALSE otherwise.
 */
static zb_ret_t check_value_conc_measurement(zb_uint16_t attr_id, zb_uint8_t endpoint, zb_uint8_t * p_value)
{
    zb_ret_t ret = ZB_FALSE;
    zb_int32_t val = ZB_ZCL_ATTR_GET32(p_value);
    zb_float32_t fval = { .v = val };
    int32_t ival = concentration_float_to_ppm(&fval);

    NRF_LOG_DEBUG("Pre-validating value %d of Concentration attribute %d", ival, attr_id);

/* Not implemented, always return true for valid attribute ids */
    switch(attr_id)
    {
        case ZB_ZCL_ATTR_CONC_MEASUREMENT_VALUE_ID:

            ret = (ival >= 0) && (ival <= 5000) ? ZB_TRUE : ZB_FALSE;
            break;

        case ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_ID:

            ret = ZB_TRUE;
            break;

        case ZB_ZCL_ATTR_CONC_MEASUREMENT_MAX_VALUE_ID:

            ret = ZB_TRUE;
            break;

        default:
           break;
    }

    return ret;
}

/**@brief Hook which is being called whenever a new value of attribute is being written.
 * 
 * @param [in] endpoint Endpoint
 * @param [in] attr_id Attribute ID
 * @param [in] new_value Pointer to the new value of the attribute
 */
static void zb_zcl_conc_measurement_write_attr_hook(zb_uint8_t endpoint, zb_uint16_t attr_id, zb_uint8_t * new_value)
{
    UNUSED_PARAMETER(new_value);

    NRF_LOG_DEBUG("Writing attribute %d of Concentration Measurement Cluster on endpoint %d", attr_id, endpoint);

    if (attr_id == ZB_ZCL_ATTR_CONC_MEASUREMENT_VALUE_ID)
    {
	      /* Implement your own write attributes hook if needed. */
    }
}

/**@brief Function which initialises the server side of Pressure Measurement Cluster. */
void zb_zcl_conc_measurement_init_server_id(zb_uint16_t cluster_id)
{
    zb_ret_t ret = zb_zcl_add_cluster_handlers(cluster_id,
                                               ZB_ZCL_CLUSTER_SERVER_ROLE,
                                               check_value_conc_measurement,
                                               zb_zcl_conc_measurement_write_attr_hook,
                                               (zb_zcl_cluster_handler_t)NULL);
    ASSERT(ret == RET_OK);
}

/**@brief Function which initialises the client side of Pressure Measurement Cluster. */
void zb_zcl_conc_measurement_init_client_id(zb_uint16_t cluster_id)
{
    zb_ret_t ret = zb_zcl_add_cluster_handlers(cluster_id,
                                               ZB_ZCL_CLUSTER_CLIENT_ROLE,
                                               check_value_conc_measurement,
                                               zb_zcl_conc_measurement_write_attr_hook,
                                               (zb_zcl_cluster_handler_t)NULL);
    ASSERT(ret == RET_OK);
}

#if 0
/**@brief Convert integer measurement value in PPM units (parts per million) to
  single precision floating point fractional value (that is the output value is the input x 1e-6) */
void concentration_ppm_to_float(const uint32_t ppm, zb_float32_t *out)
{
/*  IEEE754 float is represented as series of powers of 2. Therefore, to scale the input PPM value
    by 1e-6  we need to switch decimal fraction to power of 2 fraction.
    To do that we first multiply the PPM value by 2^20 = 1048576 and divide by 1_000_000.
    Then we subtract 20 from the exponent part in the floating point representatation.
    In the actual calculation below, we scale down both 1_000_000 and 2^20 by 2^6 to allow for larger
    PPM values without 32-bit overflow. The largest PPM value that can be safely converted
    is therefore 2^18 = 262144.
*/
    uint32_t a = (ppm * (1 << 14)) / 15625;

    int32_t upper_bit = a ? 31 - __builtin_clz(a) : 0;
/* Exponent part of the IEE754 float */
    uint32_t x = a ? (0x80 ^ (unsigned char)(upper_bit - 20 - 1)) : 0;
/* Mantissa */
    uint32_t m = a ? (a & ((1 << upper_bit) - 1)) : 0;
/* Build final output value */
    out->v = (x << 23) | (m << (23 - upper_bit));
}
#endif

