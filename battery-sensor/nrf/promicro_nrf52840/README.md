# Elfryd BLE System - nRF52840 Implementation

This repository contains the firmware for the Elfryd boat monitoring system's BLE communication layer, built for the Promicro nRF52840 board. The BLE system serves as the intermediary between sensor nodes and the nRF9160 Hub, collecting data from multiple sensors and forwarding it via I2C to the Hub for transmission to the cloud.

## System Overview

The Elfryd BLE system is part of a larger IoT system for monitoring battery and environmental conditions on boats. The BLE communication layer consists of two main components:

```
Sensor Peripherals (Zephyr) → BLE Central (TinyGo) → I2C → nRF9160 Hub → MQTT Broker
```

This repository contains both the peripheral and central components that form the BLE communication layer:

1. **BLE Peripheral (Zephyr)**: Sensor nodes that collect data (voltage, temperature, gyroscope) and advertise it over BLE
2. **BLE Central (TinyGo)**: Central node that scans for peripherals, connects to them, and forwards data to the Hub via I2C

## Project Architecture

### BLE Peripheral (Zephyr)

The peripheral firmware is built with Zephyr RTOS and uses the Bluetooth stack to expose sensor data through GATT services:

- **Battery Voltage Service**: Reports voltage readings from ADC
- **Temperature Service**: Reports temperature readings from BME280 sensor
- **Gyroscope Service**: Reports motion data from MPU6050 sensor
- **Sensor ID Service**: Provides a unique identifier for each sensor node

Each service uses custom UUIDs to identify different sensor types and ensure proper data routing.

### BLE Central (TinyGo)

The central firmware is built with TinyGo and acts as a data aggregator:

- **BLE Scanner**: Actively scans for peripherals advertising supported sensor services
- **GATT Client**: Connects to peripherals and periodically reads sensor data
- **I2C Target**: Buffers sensor data and provides it to the Hub when requested

The central component uses a modular design with separate packages for BLE communication and I2C handling.

## Key Features

- **Multi-sensor Support**: Compatible with voltage, temperature, and gyroscope sensors
- **Automatic Device Discovery**: Dynamically discovers and connects to sensor peripherals
- **ID-based Device Management**: Assigns and tracks unique IDs for each sensor
- **Efficient Power Usage**: Optimized BLE connections to minimize power consumption
- **LED Status Indication**: Visual feedback for connection status via onboard LED
- **Data Buffering**: Preserves sensor readings between Hub polling intervals
- **I2C Communication**: Standard interface for data exchange with the Hub

## Peripheral Setup

### Prerequisites

- Zephyr SDK (v3.2 or newer)
- nRF Connect SDK (v2.1 or newer)
- West build tools

### Building and Flashing

1. Navigate to the Zephyr installation directory:
```
cd ~/zephyrproject/zephyr
```

2. Build the peripheral firmware:
```
west build -b promicro_nrf52840/nrf52840/uf2 path/to/IT2901-25-elfryd/battery-sensor/nrf/promicro_nrf52840/peripheral
```

3. Flash the firmware:
```
west flash
```

Alternatively, you can flash manually by copying the `build/zephyr/zephyr.uf2` file to the device when it appears as a USB drive.

### Hardware Setup

The peripheral firmware supports the following sensors:

- **Voltage Monitoring**: Connect to AIN0 (P0.02) through a voltage divider
- **Temperature Sensing**: BME280 sensor connected via I2C0
- **Motion Detection**: MPU6050 sensor connected via I2C1

See the wiring diagram in the peripheral README for detailed connections.

## Central Setup

### Prerequisites

- TinyGo (v0.28 or newer)
- Go (v1.21 or newer)

### Building and Flashing

1. Navigate to the central directory:
```
cd path/to/IT2901-25-elfryd/battery-sensor/nrf/promicro_nrf52840/central
```

2. Build and flash for a specific sensor type:
```
tinygo flash -target=promicro-nrf52840 -ldflags="-X 'main.sensorType=Battery'" ./main
```

Replace `Battery` with `Temperature` or `Gyro` depending on which sensor data you want to collect.

## Configuration Options

### Peripheral Configuration

The peripheral behavior can be modified in `prj.conf`:

```
CONFIG_BT_DEVICE_NAME="Kystlaget Sensor"  # Bluetooth device name
CONFIG_BT_SMP=n                          # Disable pairing (simplified setup)
CONFIG_ADC=y                             # Enable ADC for voltage readings
CONFIG_SENSOR=y                          # Enable sensor drivers
CONFIG_BME280=y                          # Enable BME280 temperature sensor
CONFIG_MPU6050_TRIGGER_NONE=y            # Configure MPU6050 without interrupts
```

### Central Configuration

The central behavior is configured through command-line arguments during the build process:

- `sensorType`: Determines which type of sensors to scan for (`Battery`, `Temperature`, or `Gyro`)
- I2C address assignment is automatic based on the sensor type:
  - Battery sensors: 0x10
  - Temperature sensors: 0x20
  - Gyroscope sensors: 0x30

## Documentation

For more detailed information, refer to the specific component documentation:

- [Peripheral Documentation](./peripheral/README.md): Details on the sensor node implementation
- [Central Documentation](./central/README.md): Information on the BLE central and I2C bridge
- [I2C Shell Tool](./i2c_shell/README.md): Utility for testing I2C communication
- [Hub Documentation](../hub/README.md): nRF9160 Hub that receives data from this system
- [Broker Documentation](../../broker/README.md): MQTT broker where data is ultimately sent

## Support and Contribution

For questions or issues, please contact the contributors of the BLE system:

- David Salguero Spilde (developer)
- Jonas Elvedal Hole (developer)