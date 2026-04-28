/**
 * Implementation of ZCL concentration cluster for ZBOSS Zigbee stack in Nordic's nRF5 SDK.
 * David Shirvanyants
 * 2026/03
 */
#ifndef ZB_ZCL_ANALOG_OUTPUT_H__
#define ZB_ZCL_ANALOG_OUTPUT_H__

#include "zcl/zb_zcl_common.h"

/* Cluster ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT */
#include "zb_float.h"

/**@brief Analog Output (Basic) cluster attributes according to ZCL Specification 3.14.3.4.2. */
typedef struct
{
    zb_float32_t  present_value;
    zb_float32_t  min_present_value;
    zb_float32_t  max_present_value;
    zb_float32_t  resolution;
    uint16_t engineering_units;
} zb_zcl_analog_output_attrs_t;

/*@brief Analog Output cluster attribute identifiers
  @see ZCL spec, Analog Output Cluster 3.14.3
*/
enum zb_zcl_analog_output_attr_e
{
    /*@brief Description string, ZCL spec 3.14.11.4 */
    ZB_ZCL_ATTR_ANALOG_OUTPUT_DESCRIPTION_ID = 0x001C,
    /*@brief PresentValue, ZCL spec 3.14.11.2 */
    ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID = 0x0055,
    /*@brief MinPresentValue, ZCL spec  3.14.11.8 */
    ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_PRESENT_VALUE_ID = 0x0045,
    /*@brief MaxPresentValue, ZCL spec 3.14.11.5 */
    ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_PRESENT_VALUE_ID = 0x0041,
    /*@brief Resolution, ZCL spec 3.14.11.11 */
    ZB_ZCL_ATTR_ANALOG_OUTPUT_RESOLUTION_ID = 0x006A,
    /*@brief Units, ZCL spec 3.14.11.10 */
    ZB_ZCL_ATTR_ANALOG_OUTPUT_ENGINEERING_UNITS_ID = 0x0075,
    /*@brief Application type, ZCL spec 3.14.11.19 */
    ZB_ZCL_ATTR_ANALOG_OUTPUT_APPLICATION_TYPE_ID = 0x100,
};

/**@brief Analog Output cluster ID. */
//#define ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT                       0x000d

 /* (See: Table 3-79. Attributes of the Analog Output (Basic) Server Cluster) */

/**@brief PresentValue attribute unknown value. (float NaN) */
#define ZB_ZCL_ATTR_ANALOG_OUTPUT_VALUE_UNKNOWN                  {0x7f800001}

/**@brief MinPresentValue attribute minimum value. (float 0.0) */
#define ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_VALUE_MIN_VALUE            {0}

/**@brief MinPresentValue attribute maximum value. (float 1.0) */
#define ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_VALUE_MAX_VALUE            {0x3f800000}}

/**@brief MinPresentValue attribute invalid value. (float NaN) */
#define ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_VALUE_INVALID              {0x7f800001}

/**@brief MaxPresentValue attribute minimum value. (float 1e-45) */
#define ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_VALUE_MIN_VALUE            {0x1}

/**@brief MaxPresentValue attribute maximum value. (float 1.0) */
#define ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_VALUE_MAX_VALUE            {0x3f800000}

/**@brief MaxPresentValue attribute invalid value. (float NaN) */
#define ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_VALUE_INVALID              {0x7f800001}

/**@brief Resolution attribute minimum value. */
#define ZB_ZCL_ATTR_ANALOG_OUTPUT_RESOLUTION_MIN_VALUE            {0x1}

/**@brief Resolution attribute maximum value. */
#define ZB_ZCL_ATTR_ANALOG_OUTPUT_RESOLUTION_MAX_VALUE            {0x3f800000}

/**@brief Default value for Value attribute. */
#define ZB_ZCL_ANALOG_OUTPUT_VALUE_DEFAULT_VALUE                 {0}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID(data_ptr)    \
{                                                                              \
    ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID,                                \
    ZB_ZCL_ATTR_TYPE_SINGLE,                                                   \
    ZB_ZCL_ATTR_ACCESS_READ_WRITE | ZB_ZCL_ATTR_ACCESS_REPORTING,              \
    (void*) data_ptr                                                           \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_PRESENT_VALUE_ID(data_ptr) \
{                                                                               \
    ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_PRESENT_VALUE_ID,                             \
    ZB_ZCL_ATTR_TYPE_SINGLE,                                                    \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                               \
    (void*) data_ptr                                                            \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_PRESENT_VALUE_ID(data_ptr) \
{                                                                               \
    ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_PRESENT_VALUE_ID,                             \
    ZB_ZCL_ATTR_TYPE_SINGLE,                                                    \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                               \
    (void*) data_ptr                                                            \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ANALOG_OUTPUT_RESOLUTION_ID(data_ptr)  \
{                                                                                 \
    ZB_ZCL_ATTR_ANALOG_OUTPUT_RESOLUTION_ID,                                      \
    ZB_ZCL_ATTR_TYPE_SINGLE,                                                      \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                                 \
    (void*) data_ptr                                                              \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ANALOG_OUTPUT_ENGINEERING_UNITS_ID(data_ptr)  \
{                                                                                 \
    ZB_ZCL_ATTR_ANALOG_OUTPUT_ENGINEERING_UNITS_ID,                               \
    ZB_ZCL_ATTR_TYPE_U16,                                                         \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                                 \
    (void*) data_ptr                                                              \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ANALOG_OUTPUT_DESCRIPTION_ID(data_ptr)  \
{                                                                                 \
    ZB_ZCL_ATTR_ANALOG_OUTPUT_DESCRIPTION_ID,                                     \
    ZB_ZCL_ATTR_TYPE_CHAR_STRING,                                                 \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                                 \
    (void*) data_ptr                                                              \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ANALOG_OUTPUT_APPLICATION_TYPE_ID(data_ptr)  \
{                                                                                 \
    ZB_ZCL_ATTR_ANALOG_OUTPUT_APPLICATION_TYPE_ID,                                \
    ZB_ZCL_ATTR_TYPE_U32,                                                         \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                                 \
    (void*) data_ptr                                                              \
}

/** @brief Default value for Analog Output cluster revision global attribute */
#define ZB_ZCL_ANALOG_OUTPUT_CLUSTER_REVISION_DEFAULT ((zb_uint16_t)0x0001u)

/**@brief Declares attribute list for the Analog Output cluster on the server side.
 *
 * @param attr_list    Attribute list name.
 * @param value        Pointer to the variable to store the PresentValue attribute.
 * @param min_value    Pointer to the variable to store the MinPresentValue attribute.
 * @param max_value    Pointer to the variable to store the MaxPresentValue attribute.
 * @param resolution   Pointer to the variable to store the Resolution attribute.
 */
#define ZB_ZCL_DECLARE_ANALOG_OUTPUT_ATTRIB_LIST(attr_list,               \
    value, min_value, max_value, resolution)                                         \
  ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_ANALOG_OUTPUT) \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID, (value))                  \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_PRESENT_VALUE_ID, (min_value))          \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_PRESENT_VALUE_ID, (max_value))          \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_RESOLUTION_ID, (resolution))          \
  ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST


/**@brief Declares attribute list for the Analog Output cluster on the server side - Extended verion.
 *
 * @param attr_list    Attribute list name.
 * @param value        Pointer to the variable to store the PresentValue attribute.
 * @param min_value    Pointer to the variable to store the MinPresentValue attribute.
 * @param max_value    Pointer to the variable to store the MaxPresentValue attribute.
 * @param resolution   Pointer to the variable to store the Resolution attribute.
 * @param units        Pointer to the variable to store the Engineering Units attribute.
 * @param description  Pointer to the variable to store the Description attribute.
 * @param app_type     Pointer to the variable to store the Application Type attribute.
 */
#define ZB_ZCL_DECLARE_ANALOG_OUTPUT_ATTRIB_LIST_EX(attr_list,               \
    value, min_value, max_value, resolution, units, description, app_type)                \
  ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_ANALOG_OUTPUT)      \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID, (value))               \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_PRESENT_VALUE_ID, (min_value))       \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_PRESENT_VALUE_ID, (max_value))       \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_RESOLUTION_ID, (resolution))             \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_ENGINEERING_UNITS_ID, (units))           \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_DESCRIPTION_ID, (description))           \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ANALOG_OUTPUT_APPLICATION_TYPE_ID, (app_type))         \
  ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST


/**@brief Function initialising the server side of Analog Output Cluster. */
void zb_zcl_analog_output_init_server(void);
/**@brief Function initialising the client side of Analog Output Cluster. */
void zb_zcl_analog_output_init_client(void);

#define ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT_SERVER_ROLE_INIT zb_zcl_analog_output_init_server
#define ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT_CLIENT_ROLE_INIT zb_zcl_analog_output_init_client

#define ZB_ZCL_ANALOG_OUTPUT_CLUSTER_DESC(substance, attr_desc_list, cluster_role_mask)        \
{                                                                                     \
  (ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT),                                   \
  (ZB_ZCL_ARRAY_SIZE(attr_desc_list, zb_zcl_attr_t)),                                 \
  (attr_desc_list),                                                                   \
  (cluster_role_mask),                                                                \
  (ZB_ZCL_MANUF_CODE_INVALID),                                                        \
  (((cluster_role_mask) == ZB_ZCL_CLUSTER_SERVER_ROLE) ? zb_zcl_analog_output_init_server : \
   (((cluster_role_mask) == ZB_ZCL_CLUSTER_CLIENT_ROLE) ? zb_zcl_analog_output_init_client : NULL)) \
}

#endif /* ZB_ZCL_ANALOG_OUTPUT_H__ */
