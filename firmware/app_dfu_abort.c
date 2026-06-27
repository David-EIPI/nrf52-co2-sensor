#include "app_dfu_abort.h"

#include <stdbool.h>

#include "nrf_dfu_settings.h"
#include "nrf_log.h"
#include "zigbee_dfu_transport.h"

static volatile bool m_preserve_progress_abort;

void __real_nrf_dfu_settings_progress_reset(void);
ret_code_t __real_nrf_dfu_settings_write(nrf_dfu_flash_callback_t callback);

void __wrap_nrf_dfu_settings_progress_reset(void)
{
    if (m_preserve_progress_abort)
    {
        NRF_LOG_INFO("OTA abort: preserve DFU progress reset");
        return;
    }

    __real_nrf_dfu_settings_progress_reset();
}

ret_code_t __wrap_nrf_dfu_settings_write(nrf_dfu_flash_callback_t callback)
{
    if (m_preserve_progress_abort)
    {
        NRF_LOG_INFO("OTA abort: skip DFU settings write");
        return NRF_SUCCESS;
    }

    return __real_nrf_dfu_settings_write(callback);
}

void app_zb_abort_dfu_preserve_progress(void)
{
    m_preserve_progress_abort = true;
    zb_abort_dfu();
    m_preserve_progress_abort = false;
}
