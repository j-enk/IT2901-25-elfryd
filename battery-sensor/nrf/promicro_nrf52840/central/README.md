## TODO:
- i2c register 0 gjør ingenting nå, burde sjekke at man før en og hvis ikke så reject read. Siden hubben sender 1 til register 0 før den reader
- update testing tool til å være zephyr i2c shell eller lage oppdatere test_i2c_controller 

# Elfryd BLE Central

A TinyGo-based BLE central for the Promicro nRF52840 board that scans for, connects to, and receives data from sensor peripherals, then makes this data available to the nRF9160 Hub via I2C.

## Overview

This central firmware is a key component of the Elfryd boat monitoring system's BLE communication layer. It runs on the Promicro nRF52840 board and performs two main functions:

1. Acting as a BLE Central that scans for and connects to sensor peripherals
2. Acting as an I2C target/slave that provides collected sensor data to the nRF9160 Hub

## Features

- **BLE Scanning**: Dynamically discovers peripherals advertising supported sensor types
- **Sensor Type Filtering**: Configurable to scan for specific sensor types (Battery, Temperature, or Gyro)
- **GATT Client**: Reads data from connected peripherals at regular intervals
- **I2C Target**: Makes collected data available via I2C interface
- **Device ID Management**: Tracks unique IDs for each connected sensor
- **Concurrent Operation**: Uses goroutines for simultaneous GATT and I2C operations
- **Data Buffering**: Maintains latest readings from all connected peripherals

## System Architecture

The central firmware is structured in several packages:

- **main**: Entry point that initializes BLE and I2C subsystems
- **ble**: Handles BLE scanning, connections, and GATT client operations
- **i2c**: Implements the I2C target interface for the Hub

### Data Flow

1. **BLE Scanning**: The central scans for peripherals advertising a specific UUID based on the sensor type
2. **Connection**: Upon finding matching peripherals, it connects to them
3. **Service Discovery**: Discovers GATT services and characteristics on each peripheral
4. **Data Collection**: Periodically reads sensor data from each connected device
5. **Data Buffering**: Stores the latest reading from each peripheral in memory
6. **I2C Interface**: Provides the buffered data to the Hub when requested via I2C

## Building and Running

### Prerequisites

- TinyGo (version 0.28.0 or newer)
- Go (version 1.21 or newer)

### Installation

1. Install Go: [https://golang.org/doc/install](https://golang.org/doc/install)
2. Install TinyGo: [https://tinygo.org/getting-started/install/](https://tinygo.org/getting-started/install/)

### Building

The central firmware needs to be built with a specific sensor type:

```bash
# Build for battery sensors
tinygo build -o central_battery.uf2 -target=promicro-nrf52840 -ldflags="-X 'main.sensorType=Battery'" ./main
```

Replace `Battery` with `Temperature` or `Gyro` to build firmware for other sensor types.

### Flashing

Put the board into bootloader mode by double-pressing the reset button, then copy the generated UF2 file to the mounted drive:

```bash
# Flash directly with TinyGo
tinygo flash -target=promicro-nrf52840 -ldflags="-X 'main.sensorType=Battery'" ./main

# Or manually copy after building
cp central_battery.uf2 /path/to/mounted/PROBOOT/drive
```

## Configuration Options

### Build-time Configuration

The central is configured at build time via Go build flags:

```
-ldflags="-X 'main.sensorType=Battery'"  # Configure which sensor type to scan for
```

Available sensor types:
- `Battery`: Scan for voltage sensor peripherals
- `Temperature`: Scan for temperature sensor peripherals
- `Gyro`: Scan for gyroscope sensor peripherals

### Runtime Behavior

The following parameters are configured in the code:

- **BLE Scan Duration**: 5 seconds (in `ble/ble.go`)
- **Data Polling Interval**: 1 second (in `ble/gatt_client.go`)
- **I2C Address**: Set based on sensor type (in `main/main.go`):
  - Battery sensors: 0x10
  - Temperature sensors: 0x20
  - Gyro sensors: 0x30

## I2C Interface Specification

The central operates as an I2C target device with the following register map:

| Register | Description | Access | Format |
|----------|-------------|--------|--------|
| 0x00 | Protocol Version | Read | 1 byte (currently 0x01) |
| 0x01 | Sensor Data | Read | Variable length data block (sensor-specific format) |

### Data Format

When the I2C controller reads register 0x01, the response contains all sensor data in the following format:

- **Battery Data**: 1 byte New flag, 1 byte ID, 2 bytes Battery voltage (mV)
- **Temperature Data**: 1 byte New flag, 2 bytes Temperature (32-bit int, Celsius)
- **Gyro Data**: 1 byte New flag, 18 bytes of accelerometer and gyroscope data (3 axes each, 3 bytes per value)

Data from multiple sensors is concatenated in the response.

## Testing Tools

To test the I2C interface without the nRF9160 Hub, you can use the test controller program:

```bash
cd test_i2c_controller
tinygo flash -target=promicro-nrf52840 ./main.go
```

This program acts as an I2C controller and regularly polls the central for sensor data.

## Troubleshooting

### Common Issues

1. **No BLE devices found**: Ensure peripherals are advertising and within range
2. **I2C communication failures**: Verify correct I2C addresses and connections
3. **Missing or corrupted sensor data**: Check that peripheral firmware is correctly configured
4. **Multiple peripherals not supported**: add code on line 300 in file gattc_sd.go
```go
  dc.connectionHandle = s.connectionHandle
```

### Debugging

Monitor the USB serial output for debugging information:

```bash
minicom -D /dev/ttyACM0 -b 115200
```
or
```bash
tinygo monitor #"-port COMX" is needed if multiple devices are connected 
```

## Related Components

- [Peripheral Documentation](../peripheral/README.md): The BLE peripherals that this central connects to
- [Hub Documentation](../../hub/README.md): The nRF9160 hub that polls this central via I2C