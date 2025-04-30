## System Overview

The Sensor project is a collection of lightweight Zephyr applications that demonstrate how to interface common I²C sensors with Nordic nRF52 devices. Each application lives in its own directory (`saadc_battery_monitor/`, `bme280_temp/`, `mpu6050_gyro/`) and exposes a common sample interface so they can be reused or composed into larger projects.

Key design goals:

- **Consistency** – identical build & flash commands for every sensor.
- **Transparency** – optional shell & logging for quick bring‑up and debugging.

## Project Structure

| Directory                | Sensor/ Measure method                            
| ------------------------ | -------------------------------------------
| `saadc_battery_monitor/` | ADC battery monitor      
| `bme280_temp/`           | BME280 temperature sensor                  
| `mpu6050_gyro/`          | MPU6050 3‑axis gyroscope and accelerometer

Additional top‑level folders:

- `boards/` – custom board DTS overlays

## Getting Started

### Prerequisites

- **Zephyr SDK 0.16+**
- **west 1.2+**
- **nRF Command Line Tools**
- **Python 3.10+**
- **CMake 3.20.5+**
- **Devicetree compiler 1.4.6+**

You can get started through this guide: https://docs.zephyrproject.org/latest/develop/getting_started/index.html

### Build & Flash a Sample

```sh
eval $(zephyr-env.sh)
west build -b your battery your_pathing/bme_temp
west flash
```
Optional if west flash doesn't work:
```sh
west flash --runner nrfjprog
```

Pass `-t run` to open a serial console automatically, or use `west build -t menuconfig` to tweak sampling rate and transports.

## Project Status

| Component     | Status         |
| ------------- | -------------- |
| Battery       | ✅ Complete     |
| Temperature   | ✅ Complete     |
| Gyroscope     | ✅ Complete  |

## Development Team

For questions or issues, please contact the contributors of the sensor project:

- Leif Eggenfellner
- Sigve Fehn Kulsrud

