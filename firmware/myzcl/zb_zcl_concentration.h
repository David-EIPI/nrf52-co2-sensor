/**
 * Implementation of ZCL concentration cluster for ZBOSS Zigbee stack in Nordic's nRF5 SDK.
 * David Shirvanyants
 * 2026/03
 */
#ifndef ZB_ZCL_CONC_MEASUREMENT_H__
#define ZB_ZCL_CONC_MEASUREMENT_H__

#include "zcl/zb_zcl_common.h"

/* Cluster ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT */

#include "zb_float.h"

/**@brief Pressure Measurement cluster attributes according to ZCL Specification 4.13.1.2.1. */
typedef struct
{
    zb_float32_t  measure_value;
    zb_float32_t  min_measure_value;
    zb_float32_t  max_measure_value;
    zb_float32_t  tolerance;
} zb_zcl_conc_measurement_attrs_t;

/*@brief Pressure Measurement cluster attribute identifiers
  @see ZCL spec, Pressure Measurement Cluster 4.13.2.1
*/
enum zb_zcl_conc_measurement_attr_e
{
    /*@brief MeasuredValue, ZCL spec 4.13.2.1.1.1 */
    ZB_ZCL_ATTR_CONC_MEASUREMENT_VALUE_ID = 0x0000,
    /*@brief MinMeasuredValue, ZCL spec 4.13.2.1.1.2 */
    ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_ID = 0x0001,
    /*@brief MaxMeasuredValue, ZCL spec 4.13.2.1.1.3 */
    ZB_ZCL_ATTR_CONC_MEASUREMENT_MAX_VALUE_ID = 0x0002,
    /*@brief Tolerance, ZCL spec 4.13.2.1.1.4 */
#ifndef ZB_DISABLE_CONCENTRATION_MEASUREMENT_TOLERANCE_ID
    ZB_ZCL_ATTR_CONC_MEASUREMENT_TOLERANCE_ID = 0x0003,
#else
    ZB_ZCL_ATTR_CONC_MEASUREMENT_TOLERANCE_ID = 0xF003,
#endif
};

/**@brief Pressure measurement cluster ID. */
#define ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_CO                       0x040c
#define ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_CO2                      0x040d
#define ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_CH2                      0x040e
#define ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_H2S                      0x0411
#define ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_O2                       0x0414
#define ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_O3                       0x0415
#define ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_SO2                      0x0416

 /* (See: Table 4-47 Attributes of the Concentration Measurement server cluster) */

/**@brief MeasuredValue attribute unknown value. (float NaN) */
#define ZB_ZCL_ATTR_CONC_MEASUREMENT_VALUE_UNKNOWN                  {0x7f800001}

/**@brief MinMeasuredValue attribute minimum value. (float 0.0) */
#define ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_MIN_VALUE            {0}

/**@brief MinMeasuredValue attribute maximum value. (float 1.0) */
#define ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_MAX_VALUE            {0x3f800000}}

/**@brief MinMeasuredValue attribute invalid value. (float NaN) */
#define ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_INVALID              {0x7f800001}

/**@brief MaxMeasuredValue attribute minimum value. (float 1e-45) */
#define ZB_ZCL_ATTR_CONC_MEASUREMENT_MAX_VALUE_MIN_VALUE            {0x1}

/**@brief MaxMeasuredValue attribute maximum value. (float 1.0) */
#define ZB_ZCL_ATTR_CONC_MEASUREMENT_MAX_VALUE_MAX_VALUE            {0x3f800000}

/**@brief MaxMeasuredValue attribute invalid value. (float NaN) */
#define ZB_ZCL_ATTR_CONC_MEASUREMENT_MAX_VALUE_INVALID              {0x7f800001}

/**@brief Tolerance attribute minimum value. */
#define ZB_ZCL_ATTR_CONC_MEASUREMENT_TOLERANCE_MIN_VALUE            {0}

/**@brief Tolerance attribute maximum value. */
#define ZB_ZCL_ATTR_CONC_MEASUREMENT_TOLERANCE_MAX_VALUE            {0x3f800000}

/**@brief Default value for Value attribute. */
#define ZB_ZCL_CONC_MEASUREMENT_VALUE_DEFAULT_VALUE                 ZB_ZCL_ATTR_CONC_MEASUREMENT_VALUE_UNKNOWN

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_CONC_MEASUREMENT_VALUE_ID(data_ptr) \
{                                                                              \
    ZB_ZCL_ATTR_CONC_MEASUREMENT_VALUE_ID,                                     \
    ZB_ZCL_ATTR_TYPE_SINGLE,                                                   \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY | ZB_ZCL_ATTR_ACCESS_REPORTING,               \
    (void*) data_ptr                                                           \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_ID(data_ptr) \
{                                                                                  \
    ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_ID,                                     \
    ZB_ZCL_ATTR_TYPE_SINGLE,                                                       \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                                  \
    (void*) data_ptr                                                               \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_CONC_MEASUREMENT_MAX_VALUE_ID(data_ptr) \
{                                                                                  \
    ZB_ZCL_ATTR_CONC_MEASUREMENT_MAX_VALUE_ID,                                     \
    ZB_ZCL_ATTR_TYPE_SINGLE,                                                       \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                                  \
    (void*) data_ptr                                                               \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_CONC_MEASUREMENT_TOLERANCE_ID(data_ptr) \
{                                                                                  \
    ZB_ZCL_ATTR_CONC_MEASUREMENT_TOLERANCE_ID,                                     \
    ZB_ZCL_ATTR_TYPE_SINGLE,                                                       \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY | ZB_ZCL_ATTR_ACCESS_REPORTING,                   \
    (void*) data_ptr                                                               \
}

/** @brief Default value for Pressure Measurement cluster revision global attribute */
#define ZB_ZCL_CONC_MEASUREMENT_CLUSTER_REVISION_DEFAULT ((zb_uint16_t)0x0002u)

/**@brief Declares attribute list for the Pressure Measurement cluster on the server side.
 *
 * @param attr_list    Attribute list name.
 * @param value        Pointer to the variable to store the MeasuredValue attribute.
 * @param min_value    Pointer to the variable to store the MinMeasuredValue attribute.
 * @param max_value    Pointer to the variable to store the MaxMeasuredValue attribute.
 * @param tolerance    Pointer to the variable to store the Tolerance attribute.
 */
#define ZB_ZCL_DECLARE_CONC_MEASUREMENT_ATTRIB_LIST(attr_list,               \
    value, min_value, max_value, tolerance)                                             \
  ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_CONC_MEASUREMENT) \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_CONC_MEASUREMENT_VALUE_ID, (value))                  \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_ID, (min_value))          \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_CONC_MEASUREMENT_MAX_VALUE_ID, (max_value))          \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_CONC_MEASUREMENT_TOLERANCE_ID, (tolerance))          \
  ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

/**@brief Function initialising the server side of Pressure Measurement Cluster. */
void zb_zcl_conc_measurement_init_server_id(zb_uint16_t cluster_id);
/**@brief Function initialising the client side of Pressure Measurement Cluster. */
void zb_zcl_conc_measurement_init_client_id(zb_uint16_t cluster_id);

/**@brief Defines needed for the stack to initialise the cluster correctly. */
#define DECLARE_ZB_ZCL_CLUSTER_SERVER_ROLE_INIT(substance) \
    inline static void zb_zcl_conc_measurement_init_server_##substance(void) { \
        zb_zcl_conc_measurement_init_server_id(ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_##substance); }

#define DECLARE_ZB_ZCL_CLUSTER_CLIENT_ROLE_INIT(substance) \
    inline static void zb_zcl_conc_measurement_init_client_##substance(void) { \
        zb_zcl_conc_measurement_init_client_id(ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_##substance); }

DECLARE_ZB_ZCL_CLUSTER_SERVER_ROLE_INIT(CO)
DECLARE_ZB_ZCL_CLUSTER_CLIENT_ROLE_INIT(CO)
DECLARE_ZB_ZCL_CLUSTER_SERVER_ROLE_INIT(CO2)
DECLARE_ZB_ZCL_CLUSTER_CLIENT_ROLE_INIT(CO2)
DECLARE_ZB_ZCL_CLUSTER_SERVER_ROLE_INIT(CH2)
DECLARE_ZB_ZCL_CLUSTER_CLIENT_ROLE_INIT(CH2)
DECLARE_ZB_ZCL_CLUSTER_SERVER_ROLE_INIT(H2S)
DECLARE_ZB_ZCL_CLUSTER_CLIENT_ROLE_INIT(H2S)
DECLARE_ZB_ZCL_CLUSTER_SERVER_ROLE_INIT(O2)
DECLARE_ZB_ZCL_CLUSTER_CLIENT_ROLE_INIT(O2)
DECLARE_ZB_ZCL_CLUSTER_SERVER_ROLE_INIT(O3)
DECLARE_ZB_ZCL_CLUSTER_CLIENT_ROLE_INIT(O3)
DECLARE_ZB_ZCL_CLUSTER_SERVER_ROLE_INIT(SO2)
DECLARE_ZB_ZCL_CLUSTER_CLIENT_ROLE_INIT(SO2)


#define ZB_ZCL_CONC_CLUSTER_DESC(substance, attr_desc_list, cluster_role_mask)        \
{                                                                                     \
  (ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_##substance),                                   \
  (ZB_ZCL_ARRAY_SIZE(attr_desc_list, zb_zcl_attr_t)),                                 \
  (attr_desc_list),                                                                   \
  (cluster_role_mask),                                                                \
  (ZB_ZCL_MANUF_CODE_INVALID),                                                        \
  (((cluster_role_mask) == ZB_ZCL_CLUSTER_SERVER_ROLE) ? zb_zcl_conc_measurement_init_server_##substance : \
   (((cluster_role_mask) == ZB_ZCL_CLUSTER_CLIENT_ROLE) ? zb_zcl_conc_measurement_init_client_##substance : NULL)) \
}

#endif /* ZB_ZCL_CONC_MEASUREMENT_H__ */
