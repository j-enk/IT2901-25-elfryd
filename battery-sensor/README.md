<div id="top"></div>

<div align="center">
  <a href="https://www.kystlaget-trh.no/">
    <img src="docs/images/logo.jpg" alt="Logo" width="120" height="120">
  </a>

  <br />
  <h1 align="center">Elfryd Sensor Monitoring System</h1>
  

  <p align="center">
    A comprehensive IoT solution for monitoring battery levels and environmental conditions on the Elfryd boat, featuring secure data collection, cloud storage, and real-time monitoring capabilities.
  </p>
</div>

<hr />

<div align="center">
  <a href="https://en.wikipedia.org/wiki/C_(programming_language)"><img src="https://tinyurl.com/249mjzb9" alt="C"></a>
  <a href="https://docs.microsoft.com/en-us/dotnet/csharp/"><img src="https://tinyurl.com/2c9p4knv" alt="C#"></a>
  <a href="https://www.python.org/"><img src="https://tinyurl.com/26vvmywy" alt="Python"></a>
  <a href="https://golang.org/"><img src="https://tinyurl.com/27grvf2t" alt="Go"></a>
  <a href="https://mqtt.org/"><img src="https://tinyurl.com/2yeuyq5z" alt="MQTT"></a>
  <a href="https://www.bluetooth.com/"><img src="https://tinyurl.com/2ar74fr5" alt="Bluetooth"></a>
  <br>
  <a href="https://www.docker.com/"><img src="https://tinyurl.com/278rykn6" alt="Docker"></a>
  <a href="https://dotnet.microsoft.com/"><img src="https://tinyurl.com/26sekyup" alt=".NET"></a>
  <a href="https://azure.microsoft.com/"><img src="https://tinyurl.com/2b7fwu7m" alt="Azure"></a>
  <a href="https://vuejs.org/"><img src="https://tinyurl.com/22c28lf3" alt="Vue.js"></a>
  <a href="https://nuxtjs.org/"><img src="https://tinyurl.com/25w36sc9" alt="Nuxt.js"></a>
  

</div>

<br />

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#system-overview">System Overview</a></li>
    <li>
      <a href="#project-components">Project Components</a>
      <ul>
        <li><a href="#1-sensor-layer">Sensor Layer</a></li>
        <li><a href="#2-ble-communication-layer">BLE Communication Layer</a></li>
        <li><a href="#3-nrf9160-feather-hub">nRF9160 Feather Hub</a></li>
        <li><a href="#4-mqtt-broker-system">MQTT Broker System</a></li>
        <li><a href="#5-backend--frontend-coming-soon">Backend & Frontend</a></li>
      </ul>
    </li>
    <li><a href="#data-flow">Data Flow</a></li>
    <li><a href="#key-features">Key Features</a></li>
    <li><a href="#getting-started">Getting Started</a></li>
    <li><a href="#project-status">Project Status</a></li>
    <li><a href="#development-team">Development Team</a></li>
  </ol>
</details>

<!-- # Elfryd Boat Monitoring System -->

<!-- ![Elfryd Logo](/battery-sensor/images/logo.jpg) -->

<!-- A comprehensive IoT solution for monitoring battery levels and environmental conditions on the Elfryd boat, featuring secure data collection, cloud storage, and real-time monitoring capabilities. -->

## System Overview

The Elfryd Boat Monitoring System provides a complete data pipeline from onboard sensors to a user-friendly dashboard:

```
Sensors → BLE Network → nRF9160 Hub → MQTT Broker → Database → Backend → Frontend
```

This IoT system enables members of Kystlaget to:

- Monitor battery voltage levels across multiple batteries
- Track environmental conditions (temperature, motion)
- View historical data and trends
- Configure monitoring parameters remotely

## Project Components

### 1. Sensor Layer

Collects data from physical sensors on the boat, including:

- Battery voltage sensors
- Temperature sensors
- Gyroscope/accelerometer for motion detection

[Sensor Documentation →](/battery-sensor/nrf/sensor/README.md)

### 2. BLE Communication Layer

Enables energy-efficient local communication between sensors through Bluetooth Low Energy:

- **BLE Peripheral**: Sensor nodes that collect and transmit data
- **BLE Central**: Central node that processes incoming data from peripherals and forwards it to the nRF9160 Feather Hub

[BLE Documentation →](/battery-sensor/nrf/promicro_nrf52840/README.md)

### 3. nRF9160 Feather Hub

The central communication hub that:

- Receives sensor data from the BLE Central node
- Timestamps and processes sensor readings
- Provides LTE connectivity for cloud transmission over MQTT
- Is configurable remotely via MQTT
- Caches data to reduce network load
- Uses TLS for secure communication

[Hub Documentation →](/battery-sensor/nrf/hub/README.md)

### 4. MQTT Broker System

Azure VM-based message broker system that:

- Secures communications with TLS encryption
- Processes and stores incoming sensor data over MQTT
- Provides a REST API for data access over HTTPS
- Enables configuration commands to be sent to the boat

[Broker Documentation →](/battery-sensor/broker/README.md)

### 5. Backend

The .NET-based backend that:
- Calculates state of charge (SoC) for batteries
- Calculates pitch/roll/yaw from gyroscope data
- Provides a REST API for frontend access

[Backend Documentation →](/battery-sensor/backend/README.md)

### 6. Frontend

The Nuxt/Vue.js-based frontend that:
- Displays real-time and historical data
- Provides a user-friendly dashboard for monitoring
- Allows users to configure monitoring parameters
- Allows users to send commands to the boat

[Frontend Documentation →](/battery-sensor/frontend/README.md)

## Data Flow

1. **Data Collection**: Sensors measure battery voltage, temperature, and motion
2. **BLE Transmission**: BLE Central node receives individual readings from sensor nodes
3. **MQTT Transmission**: nRF9160 Hub sends sensor data to the MQTT broker via LTE at regular intervals
4. **Data Storage**: TimescaleDB stores time-series data efficiently
5. **Data Access**: REST API provides standardized data access for the backend
6. **Backend Processing**: Backend processes data for analysis and visualization
7. **Visualization**: Frontend dashboard displays real-time and historical data

## Key Features

- **Robust Connectivity**: Cellular LTE connection ensures data transmission from remote locations
- **Power Efficiency**: BLE communication and optimized sampling intervals preserve power
- **Data Buffering**: Continuous data collection even during network outages
- **Secure Communication**: End-to-end encryption using TLS for all data transmission
- **Configurable Parameters**: Remotely adjust sampling rates and request data between intervals
- **Time-Series Database**: Optimized storage and retrieval of sensor data

## Getting Started

For detailed setup instructions, visit the specific component documentation linked above. Each component includes its own setup guides, prerequisites, and configuration options.

### Quick Start Guides:

- [Setting up the Feather Hub](/battery-sensor/nrf/hub/docs/FLASHING.md)
- [VM Setup for Broker](/battery-sensor/broker/docs/vm_setup.md)

## Project Status

- ✅ Sensor Components
- ✅ BLE Communication
- ✅ I2C transmission
- ✅ nRF9160 Hub
- ✅ MQTT transmission
- ✅ MQTT Broker & Database
- ✅ HTTPS transmission
- ✅ Backend (.NET)
- ✅ Frontend (Nuxt/Vue.js)

## Future Work
- Make the process of adding new sensors more dynamic (updates needed for all nrf components)
- Improve the backend to support more complex calculations
- Integrate some backend endpoints with the LockBox project frontend

## Development Team

This project is developed as part of IT2901 Information Technology Project II at NTNU. Our team is focused on creating a reliable IoT monitoring solution for the Elfryd boat:

- Martin Vågseter Jakobsen 
- Magnus Hansen Åsheim
- Leif Eggenfellner
- David Salguero Spilde
- Jonas Elvedal Hole
- Sigve Fehn Kulsrud

<p align="right">(<a href="#top">back to top</a>)</p>