/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x0, LENGTH = 0xF5000
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0xa000
  uicr_bootloader_start_address (r) : ORIGIN = 0x10001014, LENGTH = 0x4
  mbr_params_page (r) : ORIGIN = 0x000F5000, LENGTH = 0x1000
  bootloader_settings_page (r) : ORIGIN = 0x000F6000, LENGTH = 0x1000
  uicr_mbr_params_page (r) : ORIGIN = 0x10001018, LENGTH = 0x4
}

INCLUDE "gcc_nrf52_common.ld"
