# Elfryd MQTT Broker System

A comprehensive IoT messaging platform with MQTT broker, TLS security, TimescaleDB storage, and a REST API for the Elfryd boat monitoring system.

## Overview

The Elfryd MQTT Broker system serves as the central data hub for the Elfryd boat monitoring project. It provides a secure and efficient way to collect, store, and access sensor data from IoT devices deployed on the Elfryd boat.

This broker system:
- Receives sensor data from boat-based IoT devices via secure MQTT
- Stores the data in a time-series database for efficient querying
- Provides a REST API for the .NET backend to retrieve and analyze the data
- Allows sending configuration commands to connected devices
- Enables real-time monitoring and historical data analysis

## System Architecture

The broker is built using a modular, containerized architecture:

1. **MQTT Broker (Eclipse Mosquitto)**: Handles secure message communication with TLS
2. **TimescaleDB**: Optimized time-series database for storing sensor readings
3. **MQTT-DB Bridge**: Processes incoming messages and stores them in the database
4. **FastAPI REST API**: Provides HTTPS access to the stored data for the backend

## Project Context

This broker component is part of the larger Elfryd system:

- **Boat Clients**: nRF-based IoT devices on boats that collect and send sensor data
- **MQTT Broker** (this repository): Central hub for receiving and storing sensor data
- **.NET Backend**: Processes the data for advanced analytics
- **Frontend**: Provides user interface for monitoring and management

## Documentation

Detailed documentation is available for all aspects of the broker system:

- [VM Setup Guide](docs/vm_setup.md): Instructions for setting up a new VM to host the broker
- [Scripts Documentation](docs/scripts.md): Details on all utility scripts and their usage
- [Bridge Documentation](docs/bridge.md): Information on the MQTT bridge and topic/message formats
- [API Documentation](docs/api.md): Comprehensive guide to the REST API endpoints

## Quick Start

To get started with the broker:

1. Set up a virtual machine using the [VM Setup Guide](docs/vm_setup.md)
2. Run the installation script: `sudo bash install.sh`
3. Download client certificates for secure connections
4. Access the API at `https://your-vm-hostname:443/health` 
5. Explore the full API documentation at `https://your-vm-hostname:443/docs`

## Data Flow

The overall data flow through the system outside the boat is as follows:

```
Boat Hub → MQTT TLS → Broker → Bridge → TimescaleDB → REST API → .NET Backend → Frontend
```

## Repository Structure

```
battery-sensor/broker/
├── app/                          # Main application directory
│   ├── docker-compose.yml        # Container orchestration
│   ├── mqtt-broker/              # MQTT broker configuration
│   ├── api/                      # API application
│   ├── bridge/                   # MQTT-DB bridge
│   └── core/                     # Shared core modules
├── docs/                         # Documentation
│   ├── api.md                    # API documentation
│   ├── bridge.md                 # Bridge documentation
│   ├── scripts.md                # Scripts documentation
│   └── vm_setup.md               # VM setup guide
├── battery_generator.sh          # Tool to generate battery test data
├── cleanup.sh                    # Clean up Docker containers and files
├── install.sh                    # Main installation script
├── db_monitor.sh                 # Tool to monitor the database
├── restart.sh                    # Restart script for services
└── seed.sh                       # Seed script for test data
```

## Security

The system implements multiple security layers:
- TLS encryption for all MQTT communications
- API key protection for sensitive API endpoints
- Docker network isolation for the database
- Self-contained certificate generation and management

## Testing

You can test the system using the provided utility scripts:
- `seed.sh`: Send sample battery data
- `battery_generator.sh`: Generate continuous battery readings
- `db_monitor.sh`: Monitor database activity in real-time

## Support and Contribution

For questions or issues, please contact the contributors of the broker system:

- Martin Vågseter Jakobsen (main developer)
- Magnus Hansen Åsheim (contributor)