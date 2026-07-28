# Full Chip Erase Recovery

This project can be recovered in two ways after a full chip erase:

- Direct boot application: fastest sanity check, no bootloader.
- Bootloader based DFU application: full installation path for OTA work.

The direct boot image starts at `0x00000000`. The bootloader based DFU image starts at `0x00001000` and requires the Adafruit bootloader for the `nice_nano` board.

## Quick Direct Boot Recovery

Build and flash the direct boot application:

```sh
make -j6 flash
```

Equivalent explicit commands:

```sh
make -j6 nrf52840_xxaa
nrfjprog -f nrf52 --eraseall
nrfjprog -f nrf52 --verify --program _build/nrf52840_xxaa.hex --sectorerase
nrfjprog -f nrf52 --reset
```

This path is useful to verify that the application itself still boots. It does not install the Adafruit bootloader and does not test bootloader based startup.

## Bootloader Based Recovery

After a full chip erase, the Adafruit bootloader must be installed first, then the DFU application image, then the Adafruit bootloader settings page must be marked valid.

If the Adafruit bootloader source is not present yet, clone it first:

```sh
git clone https://github.com/adafruit/adafruit_nrf52_bootloader Adafruit_nRF52_Bootloader
```

### 1. Build the DFU Application Artifacts

From this repository:

```sh
make -j6 dfu-package-settings
```

This produces:

- `_build/nrf52840_xxaa_dfu.hex`: application linked for bootloader start at `0x1000`
- `_build/dfu-package.zip`: serial DFU package for Adafruit bootloader
- `_build/settings.hex`: application-private Nordic DFU settings at `0xE7000`

### 2. Flash the Adafruit Bootloader

From `Adafruit_nRF52_Bootloader`, use the `nice_nano` target:

```sh
make BOARD=nice_nano flash
```

The expected bootloader image for this platform is:

```text
Adafruit_nRF52_Bootloader/_build/build-nice_nano/nice_nano_bootloader-0.10.0-13-g48a149f-dirty_nosd.hex
```

If flashing manually, use the same built image:

```sh
nrfjprog -f nrf52 --eraseall
nrfjprog -f nrf52 --verify --program Adafruit_nRF52_Bootloader/_build/build-nice_nano/nice_nano_bootloader-0.10.0-13-g48a149f-dirty_nosd.hex --sectorerase
```

The `nice_nano` bootloader image contains the MBR at `0x00000000`, bootloader code at `0x000F4000`, UICR bootloader address `0x000F4000`, and UICR MBR parameters page address `0x000FE000`.

### 3. Flash the DFU Application

Using SWD:

```sh
nrfjprog -f nrf52 --verify --program _build/nrf52840_xxaa_dfu.hex --sectorerase
```

Alternatively, use the Adafruit serial DFU path:

```sh
adafruit-nrfutil dfu serial --package _build/dfu-package.zip -p /dev/ttyACM0 --singlebank --touch 1200
```

The Makefile wrapper for serial DFU is:

```sh
make -j6 bootload SERIAL_PORT=/dev/ttyACM0
```

### 4. Mark the Application Valid for the Adafruit Bootloader

The Adafruit bootloader owns its real settings page at `0x000FF000`. Its settings page is not initialized by the bootloader hex because the linker section is `NOLOAD`.

For a freshly SWD-flashed application, write the minimum valid-app marker:

```sh
nrfjprog -f nrf52 --memwr 0x000FF000 --val 0x00000001
nrfjprog -f nrf52 --reset
```

This sets `bank_0 = BANK_VALID_APP` and `bank_0_crc = 0`. In this bootloader, CRC value `0` means "skip CRC check".

If the application was installed through `adafruit-nrfutil dfu serial`, the bootloader normally updates its own `0xFF000` settings during DFU, so this manual `memwr` step may not be needed.

### 5. Optional: Flash Application-Private DFU Settings

OTA in the application uses its own Nordic DFU settings pages, separate from the Adafruit bootloader settings:

- primary: `0x000E7000`
- backup: `0x000E6000`

If OTA is planned, flash the generated settings:

```sh
make -j6 flash-settings
```

Equivalent explicit command:

```sh
nrfjprog -f nrf52 --verify --program _build/settings.hex --sectorerase
```

These settings are for the application-side OTA logic only. They do not replace the Adafruit bootloader marker at `0xFF000`.

The direct boot application and the bootloader based DFU application reserve different private settings pages:

- Direct boot application:
  - application starts at `0x00000000`
  - private settings backup / MBR-params placeholder: `0x000F5000`
  - private settings primary: `0x000F6000`
- Bootloader based DFU application:
  - application starts at `0x00001000`
  - private settings backup: `0x000E6000`
  - private settings primary: `0x000E7000`

Do not flash `_build/settings.hex` generated for the DFU variant together with the direct boot application. It is generated for the `0xE6000`/`0xE7000` layout, not the direct boot `0xF5000`/`0xF6000` layout.

## Complete SWD Example

```sh
make -j6 dfu-package-settings
nrfjprog -f nrf52 --eraseall
nrfjprog -f nrf52 --verify --program Adafruit_nRF52_Bootloader/_build/build-nice_nano/nice_nano_bootloader-0.10.0-13-g48a149f-dirty_nosd.hex --sectorerase
nrfjprog -f nrf52 --verify --program _build/nrf52840_xxaa_dfu.hex --sectorerase
nrfjprog -f nrf52 --memwr 0x000FF000 --val 0x00000001
nrfjprog -f nrf52 --verify --program _build/settings.hex --sectorerase
nrfjprog -f nrf52 --reset
```

## Page Ownership Summary

### Bootloader Based DFU Layout

- `0x00000000`: MBR from Adafruit bootloader image
- `0x00001000`: bootloader based application start
- `0x000E6000`: application-private Nordic DFU settings backup
- `0x000E7000`: application-private Nordic DFU settings primary
- `0x000EB000..0x000F3FFF`: ZBOSS NVRAM area when `ZB_NVRAM_FLASH_AUTO_ADDRESS` is enabled with `MBR_PRESENT`
- `0x000F4000`: Adafruit bootloader start
- `0x000FE000`: MBR parameters page
- `0x000FF000`: Adafruit bootloader settings page

Do not treat `0xFF000` as the application's Nordic DFU settings. The application-side OTA settings were intentionally moved down to avoid colliding with the Adafruit bootloader and MBR pages.

### Direct Boot Layout

- `0x00000000`: direct boot application start
- `0x000F5000`: application-private DFU settings backup / MBR-params placeholder
- `0x000F6000`: application-private DFU settings primary
- `0x000F7000..0x000FFFFF`: ZBOSS NVRAM area when `ZB_NVRAM_FLASH_AUTO_ADDRESS` is enabled without `MBR_PRESENT`

There is no Adafruit bootloader, no MBR parameters page owned by the bootloader, and no Adafruit settings page in this layout.
