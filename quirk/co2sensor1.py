from zigpy.quirks.v2 import QuirkBuilder, ReportingConfig
from zigpy.zcl.clusters.general import AnalogOutput
from zha.application.platforms.number.const import NumberMode
from zigpy.quirks.v2.homeassistant.number import NumberDeviceClass
from zigpy.quirks.v2.homeassistant import EntityType
from zigpy.quirks.v2 import CustomDeviceV2

DS_CO2SENSOR_QUIRK_ID = "ds_co2sensor1_quirk"

CALIB_TIME=250

#
# Apply the quirk
#
class dsCO2SensorQuirkDevice(CustomDeviceV2):
    quirk_id = DS_CO2SENSOR_QUIRK_ID

(
    QuirkBuilder("DS", "CO2Sensor1")
    .device_class(dsCO2SensorQuirkDevice)
    .write_attr_button(
        attribute_name = "present_value",
        attribute_value = CALIB_TIME,
        cluster_id = AnalogOutput.cluster_id,
        endpoint_id = 1,
        fallback_name = "Calibrate",
        translation_key = "calibrate",
    )
    .number(
        attribute_name = "present_value",
        cluster_id = AnalogOutput.cluster_id,
        endpoint_id = 1,
        entity_type = EntityType.STANDARD,
        reporting_config=ReportingConfig(
            min_interval=10, max_interval=360, reportable_change=1
        ),
        fallback_name = "Calibration time",
        device_class = NumberDeviceClass.DURATION,
        mode = NumberMode.BOX,
    )
    .prevent_default_entity_creation(
        endpoint_id=1,
        cluster_id=AnalogOutput.cluster_id,
        unique_id_suffix=f"-{int(AnalogOutput.cluster_id)}",
    )
    .add_to_registry()
)


