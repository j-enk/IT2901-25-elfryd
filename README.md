# IT2901-25-elfryd

This repository contains two distinct IoT projects developed for Kystlaget:

## 1. Battery Sensor Monitoring System (2025)

The newest addition to this repository is a comprehensive IoT system for monitoring battery levels and environmental conditions on the Elfryd boat. This system features:

- Secure sensor data collection via BLE and MQTT
- Cloud storage with TimescaleDB
- Real-time monitoring dashboard
- Multi-layered architecture with Nordic nRF microcontrollers

For detailed information, see the [Battery Sensor README](/battery-sensor/README.md).

## 2. Digital LockBox (Legacy Project)

The original project enables controlling a lockbox through a boat booking system developed for Kystlaget. Features include:

- NFC-based key detection
- MQTT communication
- Event-based architecture
- IoT control integration with booking system

For more details on this legacy project, see the [LockBox README](/legacy/nRF91-DepositBox/README.md).

## Repository Structure

- `/battery-sensor/` - Latest IoT monitoring system (2025)
- `/legacy/` - Original digital lockbox system
