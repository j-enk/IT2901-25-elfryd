# Elfryd Hub - nRF9160 Feather Application

This repository contains the firmware for the Elfryd boat monitoring system's central hub, built for the nRF9160 Feather board. The hub serves as a bridge between local boat sensors and the Azure cloud backend, collecting data and transmitting it over LTE using MQTT.

## System Overview

The Elfryd Hub is part of a larger IoT system for monitoring battery and environmental conditions on boats. The complete data flow is:

```
Sensor Peripherals → nRF52840 BLE Central → nRF9160 Hub (this project) → MQTT Broker → Database → Backend → Frontend
```

This component (the nRF9160 Hub) receives sensor data via I2C from an nRF52840 board that acts as a BLE central node collecting data from various sensors. The hub then processes, buffers, and transmits this data to a VM-based MQTT broker over LTE.

## Project Architecture

### Thread-Based Design

The application uses a multi-threaded architecture to handle different tasks independently:

1. **MQTT Thread**: Handles LTE connection, MQTT connectivity, and message events
2. **Publisher Thread**: Manages the publishing queue and sends data to MQTT broker
3. **Sensor Thread**: Collects data from sensors and queues it for publication
4. **Time Thread**: Synchronizes time with network for accurate timestamping

### Core Features

- **Secure MQTT Communication**: TLS-secured MQTT connection with QoS 2 support
- **Configurable Sampling Rates**: Adjustable sampling intervals for each sensor type
- **Remote Configuration**: Can receive configuration commands via MQTT
- **Data Buffering**: Stores sensor readings until network connectivity is established
- **Battery Monitoring**: Supports multiple batteries (configurable number)
- **Environmental Sensors**: Temperature and gyroscope data for motion monitoring

## Configuration Options

The application is highly configurable through Kconfig options in `prj.conf`. Key configuration areas include:

### MQTT Configuration

```
CONFIG_MQTT_BUFFER_SIZE=2048            # Buffer size for MQTT messages
CONFIG_MQTT_CLIENT_ID="elfryd_hub"      # Client identifier
CONFIG_MQTT_BROKER_HOSTNAME="..."       # MQTT broker hostname
CONFIG_MQTT_BROKER_PORT=8885            # MQTT broker port
CONFIG_MQTT_TLS_SEC_TAG=42              # Security tag for TLS credentials
```

### Sensor Configuration

```
CONFIG_ELFRYD_NUM_BATTERIES=4           # Number of batteries to monitor
CONFIG_ELFRYD_MAX_BATTERY_SAMPLES=1440  # Maximum battery samples to store
CONFIG_ELFRYD_MAX_TEMP_SAMPLES=180      # Maximum temperature samples to store
CONFIG_ELFRYD_MAX_GYRO_SAMPLES=180      # Maximum gyroscope samples to store
CONFIG_ELFRYD_USE_I2C_SENSORS=n         # Use I2C sensors (y) or sample data (n)
```

### Data Transmission Intervals

```
CONFIG_SENSOR_BATTERY_INTERVAL=180      # Battery data publish interval (seconds)
CONFIG_SENSOR_TEMP_INTERVAL=180         # Temperature data publish interval (seconds)
CONFIG_SENSOR_GYRO_INTERVAL=180         # Gyroscope data publish interval (seconds)
```

## MQTT Topics and Message Formats

### Sensor Data Publishing

The application publishes to the VM broker using the following MQTT topics:

| Topic            | Message Format                                                         | Description              |
| ---------------- | ---------------------------------------------------------------------- | ------------------------ |
| `elfryd/battery` | `{battery_id}/{voltage}/{timestamp}`                                   | Battery voltage readings |
| `elfryd/temp`    | `{temperature}/{timestamp}`                                            | Temperature readings     |
| `elfryd/gyro`    | `{accel_x},{accel_y},{accel_z}/{gyro_x},{gyro_y},{gyro_z}/{timestamp}` | Gyroscope readings       |

The hub supports batch publishing of multiple readings in a single message using the pipe (`|`) character as a separator. See the [Bridge Documentation](../broker/docs/bridge.md) for more details on message formats.

### Configuration Commands

The application subscribes to the `elfryd/config/send` topic, where it can receive configuration commands over MQTT. There are two types of commands:

1. **Publish Commands**: Commands that trigger data publishing for specific sensor types
2. **Interval Commands**: Commands that change the configuration of data transmission intervals

For example, the command `temp` would request the hub to publish all available temperature data immediately, while the command `battery 10` would set the battery data publish interval to 10 seconds. More information on the available commands can be found in the [Bridge Documentation](../broker/docs/bridge.md).

Whenever a command is received and processed, the hub will publish a confirmation message back to the broker on the `elfryd/config/confirmation` topic. The message format will be the same as the received command, but since the topic is different it will not trigger the same command again.

## Data Integration

Data published by this hub is received and processed by the Elfryd MQTT Broker System, which:

- Receives the MQTT messages via a secure TLS connection
- Parses the messages into separate readings
- Stores them in a TimescaleDB time-series database
- Makes them available via a REST API for the backend system

## Building and Flashing

For instructions on building and flashing the firmware to the CircuitDojo nRF9160 Feather board, see [FLASHING.md](./docs/FLASHING.md).

## Documentation

- [Flashing Instructions](./docs/FLASHING.md): How to build and deploy the firmware
- [Broker Documentation](../broker/README.md): Details of the MQTT broker system
- [API Documentation](../broker/docs/api.md): REST API for accessing the data
- [Bridge Documentation](../broker/docs/bridge.md): MQTT bridge that processes the messages

## Support and Contribution

For questions or issues, please contact the contributors of the hub system:

- Martin Vågseter Jakobsen (developer)
