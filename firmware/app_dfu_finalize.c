#include "app_dfu_finalize.h"

#include <stdint.h>
#include <string.h>

#include "app_util_platform.h"
#include "crc32.h"
#include "nrf.h"
#include "nrf_dfu_settings.h"
#include "nrf_dfu_utils.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrfx_wdt.h"

#define APP_START_ADDR                 0x00001000UL
#define ADAFRUIT_SETTINGS_ADDR         0x000FF000UL
#define FLASH_PAGE_SIZE                0x00001000UL
#define FLASH_END_ADDR                 0x00100000UL

#define ADAFRUIT_BANK_VALID_APP        0x0001U
#define ADAFRUIT_BANK_INVALID_APP      0x00FFU

extern nrfx_wdt_channel_id m_channel_id;

typedef struct
{
    uint16_t bank_0;
    uint16_t bank_0_crc;
    uint16_t bank_1;
    uint32_t bank_0_size;
    uint32_t sd_image_size;
    uint32_t bl_image_size;
    uint32_t app_image_size;
    uint32_t sd_image_start;
} adafruit_bootloader_settings_t;

typedef struct
{
    uint32_t src_addr;
    uint32_t size;
    uint32_t crc;
    bool     ready_from_bank1;
} staged_image_info_t;

static bool image_vector_table_valid(uint32_t image_addr)
{
    uint32_t initial_sp = ((uint32_t const *)image_addr)[0];
    uint32_t reset_pc   = ((uint32_t const *)image_addr)[1];

    return ((initial_sp & 0xFF000000UL) == 0x20000000UL) &&
           (reset_pc >= APP_START_ADDR) &&
           (reset_pc < ADAFRUIT_SETTINGS_ADDR) &&
           ((reset_pc & 0x1UL) != 0);
}

static bool staged_image_info_get(staged_image_info_t * p_info, bool log_details)
{
    bool bank1_ready;
    bool progress_ready;
    bool bounds_ok;
    bool vector_ok = false;

    memset(p_info, 0, sizeof(*p_info));

    bank1_ready = (s_dfu_settings.bank_1.bank_code == NRF_DFU_BANK_VALID_APP);
    progress_ready = (s_dfu_settings.bank_current == NRF_DFU_CURRENT_BANK_1) &&
                     (s_dfu_settings.progress.firmware_image_offset != 0) &&
                     (s_dfu_settings.write_offset == s_dfu_settings.progress.firmware_image_offset);

    if (bank1_ready)
    {
        p_info->src_addr         = s_dfu_settings.progress.update_start_address;
        p_info->size             = s_dfu_settings.bank_1.image_size;
        p_info->crc              = s_dfu_settings.bank_1.image_crc;
        p_info->ready_from_bank1 = true;
    }
    else if (progress_ready)
    {
        p_info->src_addr         = nrf_dfu_bank1_start_addr();
        p_info->size             = s_dfu_settings.progress.firmware_image_offset;
        p_info->crc              = s_dfu_settings.progress.firmware_image_crc;
        p_info->ready_from_bank1 = false;
    }

    if (!bank1_ready && !progress_ready)
    {
        if (log_details)
        {
            NRF_LOG_WARNING("DFU finalize ready: reject state bank1_ready=%u progress_ready=%u",
                            bank1_ready ? 1 : 0,
                            progress_ready ? 1 : 0);
        }
        return false;
    }

    bounds_ok = !((p_info->src_addr < APP_START_ADDR) ||
                  (p_info->src_addr >= ADAFRUIT_SETTINGS_ADDR) ||
                  (p_info->size == 0) ||
                  ((p_info->src_addr + p_info->size) > ADAFRUIT_SETTINGS_ADDR) ||
                  ((APP_START_ADDR + p_info->size) > p_info->src_addr));
    if (!bounds_ok)
    {
        if (log_details)
        {
            NRF_LOG_WARNING("DFU finalize ready: reject bounds src=0x%08x size=0x%08x src_end=0x%08x app_end=0x%08x",
                            p_info->src_addr,
                            p_info->size,
                            p_info->src_addr + p_info->size,
                            APP_START_ADDR + p_info->size);
        }
        return false;
    }

    vector_ok = image_vector_table_valid(p_info->src_addr);
    if (log_details)
    {
        NRF_LOG_INFO("DFU finalize ready: vector sp=0x%08x pc=0x%08x ok=%u",
                     ((uint32_t const *)p_info->src_addr)[0],
                     ((uint32_t const *)p_info->src_addr)[1],
                     vector_ok ? 1 : 0);
    }

    return vector_ok;
}

static bool app_dfu_finalize_is_ready_log(bool log_details)
{
    staged_image_info_t image;

    return staged_image_info_get(&image, log_details);
}

bool app_dfu_finalize_is_ready(void)
{
    return app_dfu_finalize_is_ready_log(false);
}

bool app_dfu_finalize_check_and_log(void)
{
    return app_dfu_finalize_is_ready_log(true);
}

__attribute__((section(".data.ramfunc"), noinline, noreturn))
static void app_dfu_finalize_ram(uint32_t src_addr,
                                 uint32_t image_size,
                                 uint32_t wdt_channel)
{
    adafruit_bootloader_settings_t settings;
    uint32_t page_addr;
    uint32_t offset;

    settings.bank_0         = ADAFRUIT_BANK_VALID_APP;
    settings.bank_0_crc     = 0; /* Adafruit SDK11 bootloader treats 0 as "skip CRC". */
    settings.bank_1         = ADAFRUIT_BANK_INVALID_APP;
    settings.bank_0_size    = image_size;
    settings.sd_image_size  = 0;
    settings.bl_image_size  = 0;
    settings.app_image_size = 0;
    settings.sd_image_start = 0;

    __disable_irq();

    for (page_addr = APP_START_ADDR;
         page_addr < (APP_START_ADDR + image_size);
         page_addr += FLASH_PAGE_SIZE)
    {
        if (wdt_channel < 8)
        {
            NRF_WDT->RR[wdt_channel] = WDT_RR_RR_Reload;
        }

        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een;
        __asm volatile ("dsb");
        __asm volatile ("isb");
        NRF_NVMC->ERASEPAGE = page_addr;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
        {
        }
    }

    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
    __asm volatile ("dsb");
    __asm volatile ("isb");

    for (offset = 0; offset < image_size; offset += sizeof(uint32_t))
    {
        uint32_t word = 0xFFFFFFFFUL;
        uint32_t remaining = image_size - offset;

        if (wdt_channel < 8)
        {
            NRF_WDT->RR[wdt_channel] = WDT_RR_RR_Reload;
        }

        if (remaining >= sizeof(uint32_t))
        {
            word = *(uint32_t const *)(src_addr + offset);
        }
        else
        {
            uint32_t i;
            for (i = 0; i < remaining; i++)
            {
                word &= ~((uint32_t)0xFF << (i * 8));
                word |= ((uint32_t)*(uint8_t const *)(src_addr + offset + i)) << (i * 8);
            }
        }

        *(uint32_t *)(APP_START_ADDR + offset) = word;
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
        {
        }
    }

    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Een;
    __asm volatile ("dsb");
    __asm volatile ("isb");
    NRF_NVMC->ERASEPAGE = ADAFRUIT_SETTINGS_ADDR;
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
    }

    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
    __asm volatile ("dsb");
    __asm volatile ("isb");

    for (offset = 0; offset < sizeof(settings); offset += sizeof(uint32_t))
    {
        *(uint32_t *)(ADAFRUIT_SETTINGS_ADDR + offset) =
            *(uint32_t const *)((uint8_t const *)&settings + offset);
        while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
        {
        }
    }

    NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
    __asm volatile ("dsb");
    __asm volatile ("isb");

    __enable_irq();
    NVIC_SystemReset();

    while (1)
    {
    }
}

void app_dfu_finalize_and_reset(void)
{
    staged_image_info_t image;
    uint32_t crc;

    if (!staged_image_info_get(&image, true))
    {
        NRF_LOG_ERROR("DFU finalize: staged image is not ready");
        return;
    }

    NRF_LOG_INFO("DFU finalize: start src=0x%08x dst=0x%08x size=%u crc=0x%08x",
                 image.src_addr,
                 APP_START_ADDR,
                 image.size,
                 image.crc);

    crc = crc32_compute((uint8_t const *)image.src_addr, image.size, NULL);
    if (crc != image.crc)
    {
        NRF_LOG_ERROR("DFU finalize: CRC mismatch src=%08x calc=%08x expected=%08x",
                      image.src_addr,
                      crc,
                      image.crc);
        return;
    }

    NRF_LOG_INFO("DFU finalize: validation complete, copy src=0x%08x dst=0x%08x size=%u; reset follows",
                 image.src_addr,
                 APP_START_ADDR,
                 image.size);
    NRF_LOG_FINAL_FLUSH();

    app_dfu_finalize_ram(image.src_addr, image.size, m_channel_id);
}
