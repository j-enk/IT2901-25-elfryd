# I2C Shell Tool

This tool provides a command-line interface for testing and debugging I2C communication with devices connected to the Promicro nRF52840 board. It's primarily used for testing the I2C communication between the BLE Central and the nRF9160 Hub in the Elfryd boat monitoring system.

## Overview

The I2C Shell utilizes Zephyr's built-in shell commands to interact with I2C devices. It allows you to:

- Scan for I2C devices on a bus
- Read data from I2C device registers
- Write data to I2C device registers
- Troubleshoot I2C communication issues

## Prerequisites

- Zephyr SDK (v3.2 or newer)
- nRF Connect SDK (v2.1 or newer)
- West build tools
- Promicro nRF52840 development board
- I2C devices connected to the board's I2C buses

## Building and Flashing

1. Navigate to the Zephyr installation directory:
```
cd ~/zephyrproject/zephyr
```

2. Build the I2C Shell firmware:
```
west build -b promicro_nrf52840/nrf52840/uf2 path/to/IT2901-25-elfryd/battery-sensor/nrf/promicro_nrf52840/i2c_shell
```

3. Flash the firmware:
```
west flash
```

Alternatively, you can flash manually by copying the `build/zephyr/zephyr.uf2` file to the device when it appears as a USB drive.

## Usage

After flashing the firmware, connect to the board's serial port using a terminal program (e.g., PuTTY, screen, minicom) with the following settings:
- Baud rate: 115200
- Data bits: 8
- Stop bits: 1
- Parity: None
- Flow control: None

### Available I2C Commands

The I2C Shell provides the following commands:

#### Scanning for I2C Devices

To scan for all devices on an I2C bus:
```
i2c scan <i2c-device>
```

Example:
```
i2c scan i2c0
```

This command will scan the specified I2C bus and report any detected device addresses in hexadecimal format.

#### Reading from I2C Devices

To read data from an I2C device register:
```
i2c read <i2c-device> <i2c-address> <register> <amount of bytes to read>
```

Parameters:
- `<i2c-device>`: The I2C controller device name (e.g., i2c0, i2c1)
- `<i2c-address>`: The 7-bit I2C device address in hexadecimal (e.g., 0x10)
- `<register>`: The device register address to read from in hexadecimal (e.g., 0x00)
- `<amount of bytes to read>`: Number of bytes to read (decimal)

Example:
```
i2c read i2c0 0x10 0x00 4
```

This will read 4 bytes from register 0x00 of the device at address 0x10 on the i2c0 bus.

#### Writing to I2C Devices

To write data to an I2C device register:
```
i2c write <i2c-device> <i2c-address> <register> <byte0> [<byte1> ...]
```

Parameters:
- `<i2c-device>`: The I2C controller device name (e.g., i2c0, i2c1)
- `<i2c-address>`: The 7-bit I2C device address in hexadecimal (e.g., 0x10)
- `<register>`: The device register address to write to in hexadecimal (e.g., 0x00)
- `<byte0> [<byte1> ...]`: Data bytes to write in hexadecimal (e.g., 0x01 0x02)

Example:
```
i2c write i2c0 0x10 0x00 0x01 0x02 0x03 0x04
```

This will write the bytes 0x01, 0x02, 0x03, and 0x04 to register 0x00 of the device at address 0x10 on the i2c0 bus.

## Common Use Cases

### Testing the BLE Central's I2C Interface

The I2C Shell is particularly useful for testing the I2C communication between the BLE Central and the Hub. In the Elfryd system, the BLE Central operates as an I2C target (slave) at addresses:
- 0x10 for battery sensors
- 0x20 for temperature sensors
- 0x30 for gyroscope sensors

To verify communication with these devices, you can:

1. Scan for the devices:
```
i2c scan i2c0
```

2. Read sensor data:
```
i2c read i2c0 0x10 0x00 2  # Read 2 bytes of battery data
i2c read i2c0 0x20 0x00 4  # Read 4 bytes of temperature data
i2c read i2c0 0x30 0x00 6  # Read 6 bytes of gyroscope data
```

## Troubleshooting

### No Devices Found During Scan

If `i2c scan` doesn't detect any devices:
- Verify the I2C device is properly powered
- Check the I2C connections (SDA, SCL)
- Ensure pull-up resistors are properly connected
- Confirm the correct I2C bus is being used (i2c0, i2c1)

### Read/Write Operations Fail

If read or write operations return errors:
- Verify the device address is correct
- Check that the register address is valid for the target device
- Ensure the device is functioning correctly
- Verify the bus operates at a compatible speed

## Hardware Setup

On the Promicro nRF52840 board:
- I2C0 is available on pins P0.08 (SDA) and P0.11 (SCL)
- I2C1 is available on pins P0.30 (SDA) and P0.31 (SCL)

Ensure appropriate pull-up resistors (typically 4.7kΩ) are connected to both SDA and SCL lines.