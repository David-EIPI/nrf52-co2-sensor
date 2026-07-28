# Building and Flashing the Firmware

Before building, set `SDK_ROOT` in `Makefile` to the unpacked nRF5 SDK for
Thread and Zigbee 4.2.0. The build also requires the Arm GNU toolchain. The
flashing and packaging commands additionally use `nrfjprog`, `mergehex`,
`nrfutil`, and `adafruit-nrfutil`.

## Firmware variants

The Makefile produces two application variants:

| Make target | Application start | Intended use |
| --- | ---: | --- |
| `nrf52840_xxaa` | `0x00000000` | Direct boot, programmed through SWD with a J-Link probe |
| `nrf52840_xxaa_dfu` | `0x00001000` | Use with an already installed Adafruit-compatible bootloader; upload by serial DFU or as UF2 |

These images are not interchangeable. The direct-boot image includes the
vector table at address zero and does not require a bootloader. The DFU image
leaves the beginning of flash for the MBR and expects the bootloader to start
it at `0x1000`. Do not upload the direct-boot image through the bootloader.

Each application target generates these files in `_build`:

- `.out`: ELF executable containing code and debugging information.
- `.hex`: address-aware Intel HEX image used for programming and packaging.
- `.bin`: raw application bytes, mainly useful for external tools that
  explicitly require a binary image.
- `.map`: linker map, useful for inspecting code size and linked symbols.

## Direct-boot firmware

Build the image:

```sh
make -j6 nrf52840_xxaa
```

The main programming image is `_build/nrf52840_xxaa.hex`.

Connect a J-Link probe to the board's SWDIO, SWCLK, GND, and target-voltage
reference pins, then build, program, verify, and reset the MCU with:

```sh
make -j6 flash
```

The equivalent explicit commands are:

```sh
make -j6 nrf52840_xxaa
nrfjprog -f nrf52 --verify \
  --program _build/nrf52840_xxaa.hex --sectorerase
nrfjprog -f nrf52 --reset
```

`--sectorerase` preserves flash outside the sectors occupied by the HEX file.
To erase the entire MCU first, including bootloader, settings, and stored
Zigbee data, use `make erase`. A full erase is normally needed only for
recovery or when changing between incompatible flash layouts.

## Bootloader/DFU firmware

This variant requires a suitable Adafruit nRF52 bootloader to be installed
already. Building the application alone does not install that bootloader.

Build the application:

```sh
make -j6 nrf52840_xxaa_dfu
```

This creates `_build/nrf52840_xxaa_dfu.hex` and the corresponding ELF, BIN,
and map files.

### Serial DFU over USB

Create an Adafruit serial DFU package:

```sh
make -j6 dfu-package
```

The result is `_build/dfu-package.zip`. Upload it through the bootloader's USB
serial port:

```sh
make bootload SERIAL_PORT=/dev/ttyACM0
```

This runs the equivalent of:

```sh
adafruit-nrfutil --verbose dfu serial \
  --package _build/dfu-package.zip \
  -p /dev/ttyACM0 -b 115200 --singlebank --touch 1200
```

Replace `/dev/ttyACM0` with the port used by the board. The 1200-baud touch
asks a compatible bootloader to enter DFU mode. The ZIP is application-only;
it intentionally does not contain the application-side DFU settings page.

### UF2 over the mass-storage interface

Download `uf2conv.py` and its supporting files from the
[Microsoft UF2 utilities](https://github.com/microsoft/uf2/tree/master/utils).
For example, clone the UF2 repository to a directory outside this project:

```sh
git clone https://github.com/microsoft/uf2.git /path/to/uf2
```

Then create an application-only UF2 with:

```sh
python3 /path/to/uf2/utils/uf2conv.py -c -f NRF52840 \
  -o _build/nrf52840_xxaa_dfu.uf2 \
  _build/nrf52840_xxaa_dfu.hex
```

For a first installation intended to support Zigbee OTA, include the generated
DFU settings page in the UF2:

```sh
make -j6 _build/dfu_client.hex
python3 /path/to/uf2/utils/uf2conv.py -c -f NRF52840 \
  -o _build/dfu_client.uf2 \
  _build/dfu_client.hex
```

Enter the bootloader's UF2 mode, wait for its USB mass-storage volume to
appear, and copy the file to that volume:

```sh
cp _build/dfu_client.uf2 /path/to/BOOTLOADER_VOLUME/
```

The volume normally disconnects and the board resets after the copy
completes. Its mount point and the method used to enter UF2 mode depend on the
installed bootloader.

## DFU settings and OTA

The application-side DFU settings record which application is currently
valid. In particular, it stores the application's exact image size and CRC.
The Zigbee OTA code needs this baseline metadata to validate and finalize an
update correctly. A settings file must therefore be generated from the exact
DFU application HEX with which it will be used; do not reuse one from a
different build.

Generate `_build/settings.hex` with:

```sh
make -j6 _build/settings.hex
```

The Makefile currently records application version `101` in this page. Keep
that value synchronized with the firmware version when preparing a release.

For a board that already has the bootloader and DFU application installed,
flash only the application-side settings with J-Link:

```sh
make flash-settings
```

This target programs and verifies `_build/settings.hex` through SWD, then
resets the MCU. It does not replace or modify the application image.

The convenience target below builds both the serial DFU ZIP and the settings
HEX:

```sh
make -j6 dfu-package-settings
```

They remain separate artifacts: install `_build/dfu-package.zip` over serial
USB and provision `_build/settings.hex` through SWD. To perform both operations
in sequence when USB and J-Link are connected, use:

```sh
make bootload-settings SERIAL_PORT=/dev/ttyACM0
```

Alternatively, `_build/dfu_client.hex` merges the DFU application and its
settings for a single SWD or UF2 installation:

```sh
make -j6 _build/dfu_client.hex
```

Once valid settings have been provisioned, successful OTA updates maintain
their image size and CRC for subsequent updates.
