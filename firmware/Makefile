PROJECT_NAME     := zigbee_test
TARGETS          := nrf52840_xxaa nrf52840_xxaa_dfu
OUTPUT_DIRECTORY := _build
PASS_LINKER_INPUT_VIA_FILE := 0

SDK_ROOT := ${HOME}/nrf
PROJ_DIR := .

SERIAL_PORT := /dev/ttyACM0

NRFUTIL := /opt/nrf-command-line-tools/bin/nrfutil-linux

$(OUTPUT_DIRECTORY)/nrf52840_xxaa.out: \
  LINKER_SCRIPT  := gcc_nrf52.ld

$(OUTPUT_DIRECTORY)/nrf52840_xxaa_dfu.out: \
  LINKER_SCRIPT  := gcc_nrf52_dfu.ld

# Source files common to all targets
SRC_FILES += \
  $(SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_backend_uart.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_default_backends.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
  $(SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
  $(SDK_ROOT)/external/zboss/src/commissioning/bdb/bdb_finding_binding.c \
  $(SDK_ROOT)/external/zboss/src/zcl/ha_sas.c \
  $(SDK_ROOT)/external/zboss/src/zb_error/zb_error_to_string.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_attr_value.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_basic_commands.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_common.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_control4_networking.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_diagnostics_commands.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_el_measurement.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_general_commands.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_ias_zone_commands.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_identify_commands.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_main.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_nvram.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_ota_upgrade_commands.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_ota_upgrade_common.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_ota_upgrade_minimal.c \
  $(SDK_ROOT)/components/zigbee/ota/zb_zcl_ota_upgrade_addons.c \
  $(SDK_ROOT)/components/zigbee/ota/zigbee_dfu_transport.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_poll_control_commands.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_power_config_commands.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_rel_humidity.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_reporting.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_s_wwah.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_temp_measurement.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_time.c \
  $(SDK_ROOT)/external/zboss/src/zcl/zcl_wwah_common.c \
  $(SDK_ROOT)/external/zboss/src/commissioning/bdb/zdo_commissioning_bdb.c \
  $(SDK_ROOT)/components/libraries/util/app_error.c \
  $(SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
  $(SDK_ROOT)/components/libraries/scheduler/app_scheduler.c \
  $(SDK_ROOT)/components/libraries/timer/app_timer2.c \
  $(SDK_ROOT)/components/libraries/util/app_util_platform.c \
  $(SDK_ROOT)/components/libraries/assert/assert.c \
  $(SDK_ROOT)/components/libraries/timer/drv_rtc.c \
  $(SDK_ROOT)/components/libraries/util/nrf_assert.c \
  $(SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
  $(SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
  $(SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf.c \
  $(SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage.c \
  $(SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_nvmc.c \
  $(SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
  $(SDK_ROOT)/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c \
  $(SDK_ROOT)/components/libraries/queue/nrf_queue.c \
  $(SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
  $(SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
  $(SDK_ROOT)/components/libraries/sortlist/nrf_sortlist.c \
  $(SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_rng.c \
  $(SDK_ROOT)/integration/nrfx/legacy/nrf_drv_uart.c \
  $(SDK_ROOT)/modules/nrfx/hal/nrf_ecb.c \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd/nrf_nvic.c \
  $(SDK_ROOT)/modules/nrfx/hal/nrf_nvmc.c \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd/nrf_soc.c \
  $(SDK_ROOT)/modules/nrfx/soc/nrfx_atomic.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_rng.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_systick.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uart.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uarte.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_saadc.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_twi.c \
  $(SDK_ROOT)/modules/nrfx/drivers/src/nrfx_wdt.c \
  $(PROJ_DIR)/dfu_crypto_no_sign.c \
  $(PROJ_DIR)/main.c \
  $(PROJ_DIR)/app_dfu_abort.c \
  $(PROJ_DIR)/app_dfu_finalize.c \
  $(PROJ_DIR)/sensor.c \
  $(PROJ_DIR)/battvolt.c \
  $(PROJ_DIR)/myzcl/zb_zcl_concentration.c \
  $(PROJ_DIR)/myzcl/zb_zcl_analog_output.c \
  $(SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c \
  $(SDK_ROOT)/external/zboss/osif/zb_nrf52_common.c \
  $(SDK_ROOT)/external/zboss/osif/zb_nrf52_logger.c \
  $(SDK_ROOT)/external/zboss/osif/zb_nrf52_nrf_logger.c \
  $(SDK_ROOT)/external/zboss/osif/zb_nrf52_nvram.c \
  $(SDK_ROOT)/external/zboss/osif/zb_nrf52_timer.c \
  $(SDK_ROOT)/external/zboss/osif/zb_nrf52_transceiver.c \
  $(SDK_ROOT)/components/zigbee/common/zigbee_logger_eprxzcl.c \
  $(SDK_ROOT)/components/zigbee/common/zigbee_helpers.c \
\
  $(SDK_ROOT)/components/iot/background_dfu/background_dfu_block.c \
  $(SDK_ROOT)/components/iot/background_dfu/background_dfu_operation.c \
  $(SDK_ROOT)/components/iot/background_dfu/background_dfu_state.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/dfu-cc.pb.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_flash.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_handling_error.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_mbr.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_req_handler.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_settings.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_transport.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_utils.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_validation.c \
  $(SDK_ROOT)/components/libraries/bootloader/dfu/nrf_dfu_ver_validation.c \
  $(SDK_ROOT)/components/libraries/sha256/sha256.c \
  $(SDK_ROOT)/components/libraries/crc32/crc32.c \
  $(SDK_ROOT)/external/nano-pb/pb_common.c \
  $(SDK_ROOT)/external/nano-pb/pb_decode.c \


#  $(SDK_ROOT)/components/libraries/button/app_button.c \
#  $(SDK_ROOT)/components/boards/boards.c \
#  $(SDK_ROOT)/components/libraries/bsp/bsp.c \

# Include folders common to all targets
INC_FOLDERS += \
  $(SDK_ROOT)/external/zboss/src/include \
  $(SDK_ROOT)/external/fprintf \
  $(PROJ_DIR) \
  $(PROJ_DIR)/myzcl \
  $(SDK_ROOT)/integration/nrfx/legacy \
  $(SDK_ROOT)/components/libraries/experimental_section_vars \
  $(SDK_ROOT)/external/zboss/osif \
  $(SDK_ROOT)/components/libraries/atomic_fifo \
  . \
  $(SDK_ROOT)/external/nRF-IEEE-802.15.4-radio-driver/src \
  $(SDK_ROOT)/external/nRF-IEEE-802.15.4-radio-driver/src/fem/three_pin_gpio \
  $(SDK_ROOT)/components/libraries/delay \
  $(SDK_ROOT)/external/zboss/include \
  $(SDK_ROOT)/components/toolchain/cmsis/include \
  $(SDK_ROOT)/components/libraries/balloc \
  $(SDK_ROOT)/components/libraries/log \
  $(SDK_ROOT)/components/libraries/memobj \
  $(SDK_ROOT)/components/libraries/atomic \
  $(SDK_ROOT)/components \
  $(SDK_ROOT)/modules/nrfx/mdk \
  $(SDK_ROOT)/components/libraries/scheduler \
  $(SDK_ROOT)/components/libraries/strerror \
  $(SDK_ROOT)/integration/nrfx \
  $(SDK_ROOT)/modules/nrfx/drivers/include \
  $(SDK_ROOT)/components/zigbee/common \
  $(SDK_ROOT)/external/zboss/include/zcl \
  $(SDK_ROOT)/components/libraries/ringbuf \
  $(SDK_ROOT)/modules/nrfx \
  $(SDK_ROOT)/components/drivers_nrf/nrf_soc_nosd \
  $(SDK_ROOT)/components/libraries/log/src \
  $(SDK_ROOT)/external/zboss/src/zb_error \
  $(SDK_ROOT)/external/zboss/include/ha \
  $(SDK_ROOT)/external/zboss/include/osif \
  $(SDK_ROOT)/components/libraries/sortlist \
  $(SDK_ROOT)/modules/nrfx/hal \
  $(SDK_ROOT)/components/libraries/mutex \
  $(SDK_ROOT)/components/libraries/queue \
  $(SDK_ROOT)/components/libraries/pwr_mgmt \
  $(SDK_ROOT)/external/zboss/include/addons \
  $(SDK_ROOT)/components/libraries/bsp \
  $(SDK_ROOT)/components/libraries/fstorage \
  $(SDK_ROOT)/components/boards \
  $(SDK_ROOT)/components/libraries/timer \
  $(SDK_ROOT)/external/nRF-IEEE-802.15.4-radio-driver/src/fem \
  $(SDK_ROOT)/components/libraries/util \
\
  $(SDK_ROOT)/components/iot/background_dfu \
  $(SDK_ROOT)/components/iot/background_dfu/transport \
  $(SDK_ROOT)/components/libraries/bootloader/dfu \
  $(SDK_ROOT)/components/softdevice/mbr/headers \
  $(SDK_ROOT)/components/zigbee/ota \
  $(SDK_ROOT)/components/libraries/sha256 \
  $(SDK_ROOT)/external/nano-pb \
  $(SDK_ROOT)/components/libraries/bootloader \
  $(SDK_ROOT)/components/libraries/crc32 \
  $(SDK_ROOT)/components/libraries/crypto\
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310_bl \
  $(SDK_ROOT)/components/libraries/crypto/backend/cc310 \
  $(SDK_ROOT)/components/libraries/crypto/backend/mbedtls \
  $(SDK_ROOT)/components/libraries/crypto/backend/oberon \
  $(SDK_ROOT)/components/libraries/crypto/backend/micro_ecc \
  $(SDK_ROOT)/components/libraries/crypto/backend/optiga \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_sw \
  $(SDK_ROOT)/components/libraries/crypto/backend/nrf_hw \

#  $(SDK_ROOT)/components/libraries/button \

# Libraries common to all targets
LIB_FILES += \
  $(SDK_ROOT)/external/zboss/lib/gcc/libzboss.ed.a \
  $(SDK_ROOT)/external/zboss/lib/gcc/nrf52840/nrf_radio_driver.a \
  $(SDK_ROOT)/external/zboss/lib/gcc/libgppb.a \


# Optimization flags
OPT = -Os -ggdb3
# Uncomment the line below to enable link time optimization
#OPT += -flto

# C flags common to all targets
CFLAGS += $(OPT)
CFLAGS += -DAPP_TIMER_V2
CFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
CFLAGS += -DBOARD_PCA10056
CFLAGS += -DCONFIG_GPIO_AS_PINRESET
CFLAGS += -DCONFIG_ZIGBEE_ERROR_TO_STRING_ENABLED
CFLAGS += -DENABLE_FEM
CFLAGS += -DFLOAT_ABI_HARD
CFLAGS += -DLIBZBOSS_CONFIG_FILE=\"libzboss_config.ed.h\"
CFLAGS += -DNRF52840_XXAA
CFLAGS += -DZBOSS_BUILD
CFLAGS += -DZB_ED_ROLE
CFLAGS += -DZB_NRF_INTERNAL
CFLAGS += -DZB_NVRAM_FLASH_AUTO_ADDRESS
CFLAGS += -DZB_ZCL_SCENES_OPTIONAL_COMMANDS_DISABLED
CFLAGS += -DZB_ZCL_USE_ZCL7_AS_DEFAULT_REVISION
CFLAGS += -DNRF_DFU_SETTINGS_VERSION=2
CFLAGS += -DNRF_DFU_BANK1_START_ADDR=0x00075000
CFLAGS += -mcpu=cortex-m4
CFLAGS += -mthumb -mabi=aapcs
CFLAGS += -Wall -Werror
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# keep every function in a separate section, this allows linker to discard unused ones
CFLAGS += -ffunction-sections -fdata-sections -fno-strict-aliasing
CFLAGS += -fno-builtin -fshort-enums -Wno-packed-bitfield-compat
CFLAGS += -Wno-array-bounds -Wno-address-of-packed-member -Wno-unused-function

# C++ flags common to all targets
CXXFLAGS += $(OPT)
# Assembler flags common to all targets
ASMFLAGS += -ggdb3
ASMFLAGS += -mcpu=cortex-m4
ASMFLAGS += -mthumb -mabi=aapcs
ASMFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
ASMFLAGS += -DAPP_TIMER_V2
ASMFLAGS += -DAPP_TIMER_V2_RTC1_ENABLED
ASMFLAGS += -DBOARD_PCA10056
ASMFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASMFLAGS += -DENABLE_FEM
ASMFLAGS += -DFLOAT_ABI_HARD
ASMFLAGS += -DLIBZBOSS_CONFIG_FILE=\"libzboss_config.ed.h\"
ASMFLAGS += -DNRF52840_XXAA
ASMFLAGS += -DZB_ED_ROLE

# Linker flags
LDFLAGS += $(OPT)
LDFLAGS += -mthumb -mabi=aapcs -L$(SDK_ROOT)/modules/nrfx/mdk -T$(LINKER_SCRIPT)
LDFLAGS += -mcpu=cortex-m4
LDFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16
# let linker dump unused sections
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--wrap=nrf_dfu_settings_progress_reset
LDFLAGS += -Wl,--wrap=nrf_dfu_settings_write
# use newlib in nano version
LDFLAGS += --specs=nano.specs

nrf52840_xxaa: CFLAGS += -D__HEAP_SIZE=0
nrf52840_xxaa: CFLAGS += -D__STACK_SIZE=4096
# Direct-boot images have no MBR. Leave MBR_PRESENT undefined so ZBOSS
# auto-NVRAM uses UICR/full-flash fallback instead of reading app code as an
# MBR bootloader pointer. The synthetic DFU upper bound stops before the
# direct-boot app-private settings backup page at 0xF5000.
nrf52840_xxaa: CFLAGS += -DBOOTLOADER_START_ADDR=0x000F5000
nrf52840_xxaa: ASMFLAGS += -D__HEAP_SIZE=0
nrf52840_xxaa: ASMFLAGS += -D__STACK_SIZE=4096

nrf52840_xxaa_dfu: CFLAGS += -D__HEAP_SIZE=0
nrf52840_xxaa_dfu: CFLAGS += -D__STACK_SIZE=4096
# Bootloader-based images do run behind an MBR. ZBOSS auto-NVRAM can use the
# real MBR bootloader address, while Nordic DFU uses a synthetic upper bound
# before the app-private settings backup page at 0xE6000.
nrf52840_xxaa_dfu: CFLAGS += -DMBR_PRESENT
nrf52840_xxaa_dfu: CFLAGS += -DBOOTLOADER_START_ADDR=0x000E6000
nrf52840_xxaa_dfu: ASMFLAGS += -D__HEAP_SIZE=0
nrf52840_xxaa_dfu: ASMFLAGS += -D__STACK_SIZE=4096

# Add standard libraries at the very end of the linker input, after all objects
# that may need symbols provided by these libraries.
LIB_FILES += -lc -lnosys -lm -lstdc++


.PHONY: default help

# Default target - first one defined
default: nrf52840_xxaa

# Print all targets that can be built
help:
	@echo following targets are available:
	@echo		$(TARGETS)
	@echo		
	sdk_config - starting external tool for editing sdk_config.h
	@echo		flash      - flashing binary

TEMPLATE_PATH := $(SDK_ROOT)/components/toolchain/gcc


include $(TEMPLATE_PATH)/Makefile.common

$(foreach target, $(TARGETS), $(call define_target, $(target)))

.PHONY: flash erase

SDK_CONFIG_FILE := ./sdk_config.h
CMSIS_CONFIG_TOOL := $(SDK_ROOT)/external_tools/cmsisconfig/CMSIS_Configuration_Wizard.jar
sdk_config:
	java -jar $(CMSIS_CONFIG_TOOL) $(SDK_CONFIG_FILE)


$(OUTPUT_DIRECTORY)/settings.hex: $(OUTPUT_DIRECTORY)/nrf52840_xxaa_dfu.hex
	@echo Generating DFU settings for SWD provisioning
	$(NRFUTIL) settings generate --family NRF52840 --application $< --application-version 101 --bootloader-version 1 --bl-settings-version 2 --start-address 0xE7000 --app-boot-validation VALIDATE_GENERATED_CRC $@

$(OUTPUT_DIRECTORY)/dfu_client.hex: $(OUTPUT_DIRECTORY)/nrf52840_xxaa_dfu.hex $(OUTPUT_DIRECTORY)/settings.hex
	@echo Merging application with DFU settings for SWD provisioning
	mergehex -m $^ -o $@

dfu_client: $(OUTPUT_DIRECTORY)/dfu_client.hex $(OUTPUT_DIRECTORY)/settings.hex

dfu-package-settings: $(OUTPUT_DIRECTORY)/settings.hex dfu-package
	@echo Settings image for SWD: $(OUTPUT_DIRECTORY)/settings.hex
	@echo Serial DFU package remains application-only: $(OUTPUT_DIRECTORY)/dfu-package.zip

dfu-package: $(OUTPUT_DIRECTORY)/nrf52840_xxaa_dfu.hex
	@echo Packaging $<
	adafruit-nrfutil dfu genpkg --sd-req 0xFFFE --dev-type 0x0052 --application $<  $(OUTPUT_DIRECTORY)/dfu-package.zip

# Flash the direct boot image
flash: default
	@echo Flashing: $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex
	nrfjprog -f nrf52 --verify --program $(OUTPUT_DIRECTORY)/nrf52840_xxaa.hex --sectorerase
	nrfjprog -f nrf52 --reset

# Flash the settings
flash-settings: $(OUTPUT_DIRECTORY)/settings.hex
	@echo Flashing DFU settings pages only: $<
	nrfjprog -f nrf52 --verify --program $< --sectorerase
	nrfjprog -f nrf52 --reset

# Flash both
flash-s: flash flash-settings

# Flash DFU image for Adafruit bootloader
bootload: $(OUTPUT_DIRECTORY)/nrf52840_xxaa_dfu.hex dfu-package
	@echo Flashing: $<
	adafruit-nrfutil --verbose dfu serial --package $(OUTPUT_DIRECTORY)/dfu-package.zip -p $(SERIAL_PORT) -b 115200 --singlebank --touch 1200

# Flash DFU image and settings
bootload-settings: bootload flash-settings


erase:
	nrfjprog -f nrf52 --eraseall

