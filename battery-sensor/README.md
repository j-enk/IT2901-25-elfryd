# Elfryd IoT Projects

This repository contains IoT projects developed for Kystlaget to enhance their boat rental system. The projects share common infrastructure components but serve different purposes.

## Projects

### 1. [Battery Sensor System](./docs/battery-sensor-project.md)

The latest project that focuses on battery monitoring and data collection for Kystlaget's boat rental system.

- Collects and shares sensor data via Bluetooth on the boat
- Transmits sensor data over MQTT to a broker VM
- Stores data in TimescaleDB on the broker VM
- Integrates with the existing .NET backend for data access
- Provides a web-based management system for monitoring and control

[View Battery Sensor Documentation →](./battery-sensor/README.md)

### 2. [Digital Lockbox System](./docs/lockbox-project.md)

The original project that provides a digital lockbox solution for controlling access to boat keys.

- Remote locking/unlocking via nRF9160 chip
- Integration with booking system
- Access control based on booking status

[View Digital Lockbox Documentation →](./docs/lockbox-project.md)

## Shared Infrastructure

Both projects utilize shared infrastructure components for the .NET backend and frontend client:

- Frontend: React/TypeScript application (see docs [here](./server/frontend/README.md))
- Backend: ASP.NET Core API (see docs [here](./server/backend/README.md))
- Booking management and device control
