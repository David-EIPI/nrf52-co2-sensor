#ifndef _ZB_DEFS_H_
#define _ZB_DEFS_H_

#include "zb_zcl_concentration.h"
#include "zb_zcl_analog_output.h"

/* Zigbee definitions and code.
*/

#define IEEE_CHANNEL_MASK (1<<26)
//   ZB_TRANSCEIVER_ALL_CHANNELS_MASK

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE to compile air sensor (End Device) source code.
#endif

#define FIRST_ENDPOINT      1
#define SECOND_ENDPOINT     2

/* Basic cluster attributes initial values. */
#define SENSOR_INIT_BASIC_APP_VERSION        1                                    /**< Version of the application software (1 byte). */
#define SENSOR_INIT_BASIC_STACK_VERSION     10                                    /**< Version of the implementation of the Zigbee stack (1 byte). */
#define SENSOR_INIT_BASIC_HW_VERSION         1                                    /**< Version of the hardware of the device (1 byte). */
#define SENSOR_INIT_BASIC_MANUF_NAME        "\x02" "DS"                           /**< Manufacturer name (32 bytes). */
#define SENSOR_INIT_BASIC_MODEL_ID          "\x0a" "CO2Sensor1"                   /**< Model number assigned by manufacturer (32-bytes long string). */
#define SENSOR_INIT_BASIC_DATE_CODE         "\x08" "20260323"                     /**< First 8 bytes specify the date of manufacturer of the device in ISO 8601 format (YYYYMMDD). The rest (8 bytes) are manufacturer specific. */
#define SENSOR_INIT_BASIC_POWER_SOURCE      ZB_ZCL_BASIC_POWER_SOURCE_BATTERY     /**< Type of power sources available for the device. For possible values see section 3.2.2.2.8 of ZCL specification. */
#define SENSOR_INIT_BASIC_LOCATION_DESC     "\x0a" "Study room"                   /**< Describes the physical location of the device (16 bytes). May be modified during commisioning process. */
#define SENSOR_INIT_BASIC_PH_ENV            ZB_ZCL_BASIC_ENV_UNSPECIFIED          /**< Describes the type of physical environment. For possible values see section 3.2.2.2.10 of ZCL specification. */

static zb_uint8_t         m_attr_zcl_version   = ZB_ZCL_VERSION;
static zb_uint8_t         m_attr_power_source  = SENSOR_INIT_BASIC_POWER_SOURCE;
static zb_uint8_t         m_attr_manufacturer_id[] = SENSOR_INIT_BASIC_MANUF_NAME;
static zb_uint8_t         m_attr_model_id[] = SENSOR_INIT_BASIC_MODEL_ID;
static zb_uint8_t         m_attr_date_code[] = SENSOR_INIT_BASIC_DATE_CODE;
static zb_uint8_t         m_attr_location_desc[] = SENSOR_INIT_BASIC_LOCATION_DESC;
static zb_uint8_t         m_attr_phys_env = SENSOR_INIT_BASIC_PH_ENV;


/* Declare attribute list for Basic cluster (server). */
ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(basic_server_attr_list, ZB_ZCL_BASIC)
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, (&m_attr_zcl_version))
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, (&m_attr_power_source))
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, (&m_attr_manufacturer_id))
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, (&m_attr_model_id))
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_DATE_CODE_ID, (&m_attr_date_code))
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_LOCATION_DESCRIPTION_ID, (&m_attr_location_desc))
    ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_BASIC_PHYSICAL_ENVIRONMENT_ID, (&m_attr_phys_env))
ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST;

//ZB_ZCL_DECLARE_BASIC_SERVER_ATTRIB_LIST(basic_server_attr_list, &m_attr_zcl_version, &m_attr_power_source);

/* Declare attribute list for Concentration Measurement cluster */

static zb_float32_t       m_attr_conc_value = ZB_ZCL_CONC_MEASUREMENT_VALUE_DEFAULT_VALUE;
static zb_float32_t       m_attr_conc_min_value = ZB_ZCL_ATTR_CONC_MEASUREMENT_MIN_VALUE_MIN_VALUE;
static zb_float32_t       m_attr_conc_max_value = ZB_ZCL_ATTR_CONC_MEASUREMENT_MAX_VALUE_MAX_VALUE;
static zb_float32_t       m_attr_conc_tolerance = ZB_ZCL_ATTR_CONC_MEASUREMENT_TOLERANCE_MIN_VALUE;

ZB_ZCL_DECLARE_CONC_MEASUREMENT_ATTRIB_LIST(conc_measure_attr_list,
    &m_attr_conc_value,
    &m_attr_conc_min_value,
    &m_attr_conc_max_value,
    &m_attr_conc_tolerance);

/* Declare attribute list for Electrical Measurement cluster */

static uint32_t m_el_measurement_type = ZB_ZCL_ELECTRICAL_MEASUREMENT_DC_MEASUREMENT;
static int16_t m_dc_voltage = 0;
static uint16_t m_dc_voltage_mult = 1;
static uint16_t m_dc_voltage_div = 1000; /* Voltage will be reported in mV */

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_ID(data_ptr) \
{ \
  ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_ID, \
  ZB_ZCL_ATTR_TYPE_S16, \
  ZB_ZCL_ATTR_ACCESS_READ_ONLY | ZB_ZCL_ATTR_ACCESS_REPORTING, \
  (void*) data_ptr \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MULTIPLIER_ID(data_ptr) \
{                                                                                                 \
  ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MULTIPLIER_ID,                                    \
  ZB_ZCL_ATTR_TYPE_U16,                                                                           \
  ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                                                   \
  (void*) data_ptr                                                                                \
}

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_DIVISOR_ID(data_ptr) \
{                                                                                                 \
  ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_DIVISOR_ID,                                       \
  ZB_ZCL_ATTR_TYPE_U16,                                                                           \
  ZB_ZCL_ATTR_ACCESS_READ_ONLY,                                                                   \
  (void*) data_ptr                                                                           \
}

ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(el_measure_attr_list, ZB_ZCL_ELECTRICAL_MEASUREMENT)
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_MEASUREMENT_TYPE_ID, (&m_el_measurement_type))
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_ID, (&m_dc_voltage))
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_MULTIPLIER_ID, (&m_dc_voltage_mult))
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_ELECTRICAL_MEASUREMENT_DC_VOLTAGE_DIVISOR_ID, (&m_dc_voltage_div))
ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST;


/* Declare attribute list for Relative Humidity Measurement cluster */

static uint32_t m_attr_hum_value     = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_UNKNOWN;
static uint16_t m_attr_hum_min_value = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_MIN_VALUE;
static uint16_t m_attr_hum_max_value = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MAX_VALUE_MAX_VALUE;

ZB_ZCL_DECLARE_REL_HUMIDITY_MEASUREMENT_ATTRIB_LIST(rel_hum_attr_list,
    &m_attr_hum_value,
    &m_attr_hum_min_value,
    &m_attr_hum_max_value);


/* Declare attribute list for Temperature Measurement cluster */

static int32_t m_attr_temp_value     = ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_UNKNOWN;
static int16_t m_attr_temp_min_value = ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_MIN_VALUE;
static int16_t m_attr_temp_max_value = ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_MAX_VALUE;
static uint32_t m_attr_temp_tolerance = ZB_ZCL_ATTR_TEMP_MEASUREMENT_TOLERANCE_MIN_VALUE;


ZB_ZCL_DECLARE_TEMP_MEASUREMENT_ATTRIB_LIST(temp_attr_list,
    &m_attr_temp_value,
    &m_attr_temp_min_value,
    &m_attr_temp_max_value,
    &m_attr_temp_tolerance);


static zb_float32_t m_calib_dur = ZB_ZCL_ANALOG_OUTPUT_VALUE_DEFAULT_VALUE;
static zb_float32_t m_calib_dur_min_value = (zb_float32_t)(0.0f);
static zb_float32_t m_calib_dur_max_value = (zb_float32_t)(10000.0f);
static zb_float32_t m_calib_dur_resolution = (zb_float32_t)(1.0f);
static uint16_t m_calib_dur_units = 73; /* BACnet: seconds */;
static char calib_dur_description[] = "\x0b" "Calibration";
static uint32_t m_calib_dur_app_type = 0x05 << 16; /* Type: Parts per million */;

/* Declare attribute list for Analog Output cluster - calibration duration */
ZB_ZCL_DECLARE_ANALOG_OUTPUT_ATTRIB_LIST_EX(calib_dur_attr_list,
    &m_calib_dur,
    &m_calib_dur_min_value,
    &m_calib_dur_max_value,
    &m_calib_dur_resolution,
    &m_calib_dur_units,
    &calib_dur_description,
    &m_calib_dur_app_type);


static zb_float32_t m_calib_conc = (zb_float32_t)(420.0f);
static zb_float32_t m_calib_conc_min_value = (zb_float32_t)(300.0f);
static zb_float32_t m_calib_conc_max_value = (zb_float32_t)(2000.0f);
static zb_float32_t m_calib_conc_resolution = (zb_float32_t)(1.0f);
static uint16_t m_calib_conc_units = 96; /* BACnet: parts per million */
static char calib_conc_description[] = "\x0f" "CO2 target conc.";
static uint32_t m_calib_conc_app_type = 14 << 16; /* Type: Time in seconds */;

/* Declare attribute list for Analog Output cluster - target concentration */
ZB_ZCL_DECLARE_ANALOG_OUTPUT_ATTRIB_LIST_EX(calib_conc_attr_list,
    &m_calib_conc,
    &m_calib_conc_min_value,
    &m_calib_conc_max_value,
    &m_calib_conc_resolution,
    &m_calib_conc_units,
    &calib_conc_description,
    &m_calib_conc_app_type);


zb_zcl_cluster_desc_t sensor_cluster_list[] =
{
  ZB_ZCL_CLUSTER_DESC(
    ZB_ZCL_CLUSTER_ID_BASIC,
    ZB_ZCL_ARRAY_SIZE(basic_server_attr_list, zb_zcl_attr_t),
    (basic_server_attr_list),
    ZB_ZCL_CLUSTER_SERVER_ROLE,
    ZB_ZCL_MANUF_CODE_INVALID
  ),
  ZB_ZCL_CLUSTER_DESC(
    ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT,
    ZB_ZCL_ARRAY_SIZE(el_measure_attr_list, zb_zcl_attr_t),
    (el_measure_attr_list),
    ZB_ZCL_CLUSTER_SERVER_ROLE,
    ZB_ZCL_MANUF_CODE_INVALID
  ),
  ZB_ZCL_CLUSTER_DESC(
    ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT,
    ZB_ZCL_ARRAY_SIZE(rel_hum_attr_list, zb_zcl_attr_t),
    (rel_hum_attr_list),
    ZB_ZCL_CLUSTER_SERVER_ROLE,
    ZB_ZCL_MANUF_CODE_INVALID
  ),
  ZB_ZCL_CLUSTER_DESC(
    ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT,
    ZB_ZCL_ARRAY_SIZE(temp_attr_list, zb_zcl_attr_t),
    (temp_attr_list),
    ZB_ZCL_CLUSTER_SERVER_ROLE,
    ZB_ZCL_MANUF_CODE_INVALID
  ),
  ZB_ZCL_CONC_CLUSTER_DESC(
    CO2,
    (conc_measure_attr_list),
    ZB_ZCL_CLUSTER_SERVER_ROLE
  ),
  ZB_ZCL_CLUSTER_DESC(
    ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT,
    ZB_ZCL_ARRAY_SIZE(calib_conc_attr_list, zb_zcl_attr_t),
    (calib_conc_attr_list),
    ZB_ZCL_CLUSTER_SERVER_ROLE,
    ZB_ZCL_MANUF_CODE_INVALID
  ),
};


zb_zcl_cluster_desc_t config_cluster_list[] =
{
  ZB_ZCL_CLUSTER_DESC(
    ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT,
    ZB_ZCL_ARRAY_SIZE(calib_dur_attr_list, zb_zcl_attr_t),
    (calib_dur_attr_list),
    ZB_ZCL_CLUSTER_SERVER_ROLE,
    ZB_ZCL_MANUF_CODE_INVALID
  ),
};

/* Cluster counts have to be plain numbers, they will be stringized later in the _DESC macros. */

#define SENSOR_CLUSTER_LIST_IN_NUM    6
#define SENSOR_CLUSTER_LIST_OUT_NUM   0

#define CONFIG_CLUSTER_LIST_IN_NUM    1
#define CONFIG_CLUSTER_LIST_OUT_NUM   0

/* Reporting clusters list. Currently there are 6 reporting clusters.
 Only one reporting context needs to be allocated between all endpoints.
*/
#define ZB_SENSOR_REPORT_ATTR_COUNT   6

/* Declare cluster list description type. All clusters are of type in (server). */
/* Cluster list description for the first endpoint */
#define ZB_ZCL_DECLARE_AIR_SENSOR_SIMPLE_DESC(ep_name, ep_id, in_clust_num, out_clust_num)             \
  ZB_DECLARE_SIMPLE_DESC_VA(in_clust_num, out_clust_num, ep_name);                          \
  ZB_AF_SIMPLE_DESC_TYPE_VA(in_clust_num, out_clust_num, ep_name) simple_desc_##ep_name =   \
  { \
    FIRST_ENDPOINT, \
    ZB_AF_HA_PROFILE_ID, \
    ZB_HA_SIMPLE_SENSOR_DEVICE_ID, \
    0, \
    0, \
    (in_clust_num + out_clust_num), \
    0, \
    { \
        ZB_ZCL_CLUSTER_ID_BASIC, \
        ZB_ZCL_CLUSTER_ID_CONC_MEASUREMENT_CO2, \
        ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT, \
        ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, \
        ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, \
        ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT, \
    }, \
  };

/* Cluster list description for the first endpoint */
#define ZB_ZCL_DECLARE_CONFIG_SIMPLE_DESC(ep_name, ep_id, in_clust_num, out_clust_num)             \
  ZB_DECLARE_SIMPLE_DESC_VA(in_clust_num, out_clust_num, ep_name);                          \
  ZB_AF_SIMPLE_DESC_TYPE_VA(in_clust_num, out_clust_num, ep_name) simple_desc_##ep_name =   \
  { \
    SECOND_ENDPOINT, \
    ZB_AF_HA_PROFILE_ID, \
    ZB_HA_SIMPLE_SENSOR_DEVICE_ID, \
    0, \
    0, \
    (in_clust_num + out_clust_num), \
    0, \
    { \
        ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT, \
    }, \
  };

/**@brief Declare Air Sensor endpoint.
 *
 * @param[IN] ep_name                endpoint variable name.
 * @param[IN] ep_id [IN]             endpoint ID.
 * @param[IN] cluster_list [IN]      list of endpoint clusters
 */
#define ZB_ZCL_DECLARE_AIR_SENSOR_EP(ep_name, ep_id)        \
    ZB_ZCL_DECLARE_AIR_SENSOR_SIMPLE_DESC(ep_name, \
        ep_id, \
        SENSOR_CLUSTER_LIST_IN_NUM, \
        SENSOR_CLUSTER_LIST_OUT_NUM); \
    ZBOSS_DEVICE_DECLARE_REPORTING_CTX(sensor_reporting_clusters, ZB_SENSOR_REPORT_ATTR_COUNT); \
\
    ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id, \
      ZB_AF_HA_PROFILE_ID, \
      0, \
      NULL, \
      ZB_ZCL_ARRAY_SIZE(sensor_cluster_list, zb_zcl_cluster_desc_t), \
      sensor_cluster_list, \
      (zb_af_simple_desc_1_1_t*)&simple_desc_##ep_name, \
      ZB_SENSOR_REPORT_ATTR_COUNT-1, sensor_reporting_clusters, 0, NULL)


#define ZB_ZCL_DECLARE_CONFIG_EP(ep_name, ep_id)        \
    ZB_ZCL_DECLARE_CONFIG_SIMPLE_DESC(ep_name, \
        ep_id, \
        CONFIG_CLUSTER_LIST_IN_NUM, \
        CONFIG_CLUSTER_LIST_OUT_NUM); \
    ZB_AF_DECLARE_ENDPOINT_DESC(ep_name, ep_id, \
      ZB_AF_HA_PROFILE_ID, \
      0, \
      NULL, \
      ZB_ZCL_ARRAY_SIZE(config_cluster_list, zb_zcl_cluster_desc_t), \
      config_cluster_list, \
      (zb_af_simple_desc_1_1_t*)&simple_desc_##ep_name, \
      1, sensor_reporting_clusters+5,            \
      0, NULL)


#define SCD40_CONC_MEASUREMENT_UNKNOWN ((uint32_t)-1)

#define SCD40_SENSOR_OUTPUT_UNKNOWN { \
  .ppm_CO2 = SCD40_CONC_MEASUREMENT_UNKNOWN, \
  .c_temperature = ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_UNKNOWN, \
  .p_humidity = ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_UNKNOWN, \
 }


#endif
