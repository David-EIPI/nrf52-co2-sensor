/**
 * Implementation of ZCL concentration cluster for ZBOSS Zigbee stack in Nordic's nRF5 SDK.
 * David Shirvanyants
 * 2026/03
 */
#include "zboss_api.h"
#include "zb_zcl_analog_output.h"
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
static zb_ret_t check_value_analog_output(zb_uint16_t attr_id, zb_uint8_t endpoint, zb_uint8_t * p_value)
{
    zb_ret_t ret = ZB_FALSE;
    zb_int32_t val = ZB_ZCL_ATTR_GET32(p_value);
    zb_float32_t fval = { .v = val };
    int32_t int_val = float_to_int32(&fval);

    NRF_LOG_DEBUG("Pre-validating value %hi of Present Value attribute %d", int_val, attr_id);

/* Not implemented, always return true for valid attribute ids */
    switch(attr_id)
    {
        case ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID:
            ret = ZB_TRUE;
            break;

        case ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_PRESENT_VALUE_ID:

            ret = ZB_TRUE;
            break;

        case ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_PRESENT_VALUE_ID:

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
static void zb_zcl_analog_output_write_attr_hook(zb_uint8_t endpoint, zb_uint16_t attr_id, zb_uint8_t * new_value)
{
    UNUSED_PARAMETER(new_value);

    NRF_LOG_DEBUG("Writing attribute %d of Analog Output Cluster on endpoint %d", attr_id, endpoint);

    if (attr_id == ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID)
    {
	      /* Implement your own write attributes hook if needed. */
    }
}

/**@brief Function which initialises the server side of Pressure Measurement Cluster. */
void zb_zcl_analog_output_init_server(void)
{
    zb_ret_t ret = zb_zcl_add_cluster_handlers(ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT,
                                               ZB_ZCL_CLUSTER_SERVER_ROLE,
                                               check_value_analog_output,
                                               zb_zcl_analog_output_write_attr_hook,
                                               (zb_zcl_cluster_handler_t)NULL);
    ASSERT(ret == RET_OK);
}

/**@brief Function which initialises the client side of Pressure Measurement Cluster. */
void zb_zcl_analog_output_init_client_id(void)
{
    zb_ret_t ret = zb_zcl_add_cluster_handlers(ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT,
                                               ZB_ZCL_CLUSTER_CLIENT_ROLE,
                                               check_value_analog_output,
                                               zb_zcl_analog_output_write_attr_hook,
                                               (zb_zcl_cluster_handler_t)NULL);
    ASSERT(ret == RET_OK);
}


